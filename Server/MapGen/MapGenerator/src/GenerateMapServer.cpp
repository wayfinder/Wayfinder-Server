/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSMap.h"
#include "OldOverviewMap.h"
#include <iostream>
#include <iomanip>
#include "MC2String.h"
#include <fstream>
#include "Properties.h"
#include "DataBuffer.h"
#include "DatagramSocket.h"
#include "multicast.h"
#include "CoordinateTransformer.h"
#include "ExtraDataReader.h"
#include "OldExtraDataUtility.h"
#include "StringTable.h"
#include "StringUtility.h"
#include "GMSCountryOverviewMap.h"
#include "Crossing.h"
#include "GMSUtility.h"
#include "IDPairVector.h"
#include "CommandlineOptionHandler.h"
#include "GMSMidMifHandler.h"

#include "OldBuiltUpAreaItem.h"
#include "OldMunicipalItem.h"
#include "OldPointOfInterestItem.h"
#include "OldStreetSegmentItem.h"
#include "OldWaterItem.h"

#include <util/PlatformUtils.hpp>
#include "XMLParserHelper.h"
#include "XMLIndata.h"

#include "GfxUtility.h"
#include "OldExternalConnections.h"
#include "NationalProperties.h"
#include "CharEncoding.h"
#include "MapGenUtil.h"
#include "OldMapIdByName.h"
#include "MapIndex.h"
#include "GMSTool.h"
#include "STLStringUtility.h"
#include "AddrPointNames.h"
#include "GMSPolyUtility.h"
#include "GenericMap.h"
#include "MapBits.h"

#include "SysUtility.h"
#include "Utility.h"



// The types of supported inputfiles
//------------------------------------------------------------------------
   enum readFile_t {
      gdf, 
      mcm,
      xml,
      UNKNOWN
   };

// Commandline parameters
//------------------------------------------------------------------------
uint32   CL_verboseLevel               = 0;
bool     CL_regenerateTurnDescriptions = false;
bool     CL_setLocations               = false;
uint32   CL_saveAsID                   = MAX_UINT32;
char*    CL_readExtraData              = NULL;
bool     CL_driveOnLeftSide            = false;
char*    CL_country                    = NULL;
bool     CL_extconnections             = false;
bool     CL_processNeighbours          = false;
bool     CL_addItemsFromExtraItemMaps  = false;
bool     CL_createCountryMap           = false; 
char*    CL_addZipToCountryMap         = NULL;
char*    CL_createItemsFromMidMif      = NULL;
bool     CL_useCoordToFindCorrectMap   = false;
char*    CL_initMapFromMidMif          = NULL;
bool     CL_generateStreetsFromStreetSegments = false;
bool     CL_mixedPostProcessing        = false;
uint32   CL_startAtMap                 = 0;
uint32   CL_endAtMap                   = MAX_UINT32;
char*    CL_name                       = NULL;
char*    CL_maporigin                  = NULL;
char*    CL_printMidMif                = NULL;
bool     CL_checkExtraData             = false;
char*    CL_printMidMifMapGfx          = NULL;
bool     CL_applyExtraDataOnMergedMaps = false;
char*    CL_applyWaspOnMergedMaps      = NULL;
char*    CL_getEDtimePerCountry        = NULL;
bool     CL_filterCoordinates          = false;
bool     CL_filterLevels               = false;
bool     CL_filterCountryPolygon       = false;
char*    CL_countryPolygonBreakPoints  = NULL;
char*    CL_copyFilteringFromMap       = NULL;
char*    CL_copyrightString            = NULL;
bool     CL_updateNodeLevelsInMap      = false;
bool     CL_createBorderBoundrySegments = false;
bool     CL_gfxFilteringForCoPols      = false;
bool     CL_createBorderItems          = false;
bool     CL_createNewIndex             = false;
char*    CL_coXMLFilesStr              = NULL;
char*    CL_cooXMLFilesStr             = NULL;
bool     CL_addRegionsToIndex          = false;
bool     CL_addMapSupCoverage          = false;
char*    CL_crbXMLFilesStr             = NULL;
char*    CL_mapSupNamesXMLFilesStr     = NULL;
char*    CL_arXMLFilesStr              = NULL;
bool     CL_updateCreationTimes        = false;
bool     CL_createOverview             = false;
bool     CL_printHeader                = false;
uint32   CL_relabel[2]; // initiated to MAX_UINT32 first in main.
bool     CL_usingDatabase              = false;
char*    CL_addSectionedZipPOIs        = NULL;
bool     CL_addAllMidMifItems          = false;
char*    CL_tool                       = NULL;
char*    CL_addrPointFile              = NULL;
bool     CL_listIndex                  = false;
uint32   CL_m3MapID                    = MAX_UINT32;
bool     CL_finishIndexAreas           = false;
bool     CL_loopAllMaps                = false;
bool     CL_tryToBuildMapGfxFromMunicipals = false;
char*    CL_removeDupItemsFromMidMif   = NULL;
typedef map<uint32, map<uint32, GMSMap::extdata_t>* > danglingEndsByMap_t;

// Forward function declarations
//------------------------------------------------------------------------
void initCommandline(CommandlineOptionHandler &);
bool commandLineSemantics(CommandlineOptionHandler &,
                          StringTable::countryCode &);
bool fileOk(MC2String &, readFile_t &);
void updateExternalConnections();
void updExtConnsOfMap(OldGenericMap* curMap, 
                      danglingEndsByMap_t& danglingEndsByMap,
                      multimap<uint32, uint32>& surroundingMapsByMap );
void createCountryMaps( CommandlineOptionHandler* coh,
                        map<MC2String, MC2String>& overviewByUnderview );
void createCountryMapBorder( CommandlineOptionHandler* coh, 
                             GMSCountryOverviewMap* curCountryMap);
bool processXMLData( CommandlineOptionHandler* coh );
bool addExtraItems( CommandlineOptionHandler* coh );
bool readExtraData();
bool mixedPostProcessing(GMSMap* theMap);

bool applyWASPChangesOnMergedMaps(const char* filename);
bool applyChangesFromUnderviewsToOverviews( multimap<uint32, uint32> 
                                            itemsAffectedInUVMaps);
bool getExtradataTimePerCountry(const char* fileName);
bool setNativeLanguages(OldGenericMap* theMap);
bool createGfxFilteringForCountryPolygons();
bool createGfxFilteringForOneCountryPolygon( GMSCountryOverviewMap* coMap);
bool filterCoPolCoordinateLevels( OldGenericMap* theMap, 
                                  const char* breakPointsFileName );
void createNewIndex(char* coXMLFilesStr, 
                    const char* cooXMLFilesStr);
void listIndex();
void addRegions(char* arXMLFilesStr, MapIndex& mapIndex);
void addMapSupCoverage(MapIndex& mapIndex,                       
                       char* mapSupCovXmlFile, 
                       char* mapSupNameXmlFile);
void updateCreationTimes();
void createOverview(char* coXMLFilesStr);
bool changeMapID(uint32 oldID, uint32 newID);
void createBorderItemsInCoMaps( const char* mapPath );
bool createBorderItemsInOneCoMap( GMSCountryOverviewMap* coMap );
void addAddrPointNames(char* addrPointFile);
void finishIndexAreas();
void removeDupItemsFromMidMif();
//
// Main
//========================================================================
int main(int argc, char **argv) {
  
   CL_relabel[0]=MAX_UINT32;
   CL_relabel[1]=MAX_UINT32;
   // some validation code.
   if (strcmp(
      "XX",
      StringTable::countryCodes[StringTable::NBR_COUNTRY_CODES][0]) != 0)
   {
      mc2log << error
             << "countryCodes table not up to date with countryCode enum."
             << endl;
      exit(1);
   }






   CommandlineOptionHandler coh(argc, argv);
   initCommandline(coh);

   int returnCode = 0;

   if (!coh.parse()) {
      exit(1);
   }

   StringTable::countryCode countryCode;
   if (!commandLineSemantics(coh, countryCode)) {
      exit(1);
   }
   
   // parse the mc2.prop file to find out some properties
   if (!Properties::setPropertyFileName(coh.getPropertyFileName())) {
      cerr << "No such file or directory: '"
           << coh.getPropertyFileName() << "'" << endl;
      exit(1);
   }

   // Check if to do anything with all the maps
   if (CL_extconnections) {
      // Check and update the external connections
      updateExternalConnections();
      exit(0);
      
   } else if (CL_createCountryMap) {
      // Information about which maps that should make up which
      // country map is stored in xml-files. The xml-files are submitted
      // to the tail of the command line.
      processXMLData( &coh ); // includes call to createCountryMaps
      
   } else if (CL_addItemsFromExtraItemMaps) {
      addExtraItems( &coh );
      exit(0);

   } else if (CL_readExtraData != NULL) {
      readExtraData();
      exit(0);

   } else if (CL_addrPointFile != NULL) {
      addAddrPointNames(CL_addrPointFile);
      exit(0);
      
   } else if (CL_finishIndexAreas) {
      finishIndexAreas();
      exit(0);
      
   } else if (CL_applyWaspOnMergedMaps != NULL) {
      applyWASPChangesOnMergedMaps( CL_applyWaspOnMergedMaps );
      exit(0);
      
   } else if (CL_getEDtimePerCountry != NULL) {
      getExtradataTimePerCountry( CL_getEDtimePerCountry );
      exit(0);
   }
   
   else if ( CL_gfxFilteringForCoPols && (coh.getTailLength() == 0) ) {
      mc2dbg << "createGfxFilteringForCountryPolygons: loop all maps in "
             << "current directory" << endl;
      bool ok = createGfxFilteringForCountryPolygons();
      if ( ! ok ) {
         exit(1);
      }
      exit(0);
   }
   else if ( CL_createBorderItems && (coh.getTailLength() == 0)) {
      const char* mapPath = "./";
      mc2dbg << "createBorderItemsInCoMaps: loop all maps in "
             << mapPath << endl;
      createBorderItemsInCoMaps( mapPath );
      exit(0);
   }
   else if (CL_createNewIndex){
      createNewIndex(CL_coXMLFilesStr, CL_cooXMLFilesStr);
      exit(0);
   }
   else if (CL_listIndex){
      listIndex();
   }
   else if (CL_m3MapID != MAX_UINT32){
      uint32 mapIDWithMapSet = 
         MapBits::getMapIDWithMapSet( CL_m3MapID, Properties::getMapSet() );
      MC2String mapFileName = 
         GenericMapHeader::getFilenameFromID(mapIDWithMapSet);
      mc2log << info << "Loading map file name: " << mapFileName << endl;
      GenericMap* theMap = GenericMap::createMap( mapIDWithMapSet );
      delete theMap;
   }
   else if (CL_addRegionsToIndex){
      // Initialize the XML system
      try { XMLPlatformUtils::Initialize(); } 
      catch(const XMLException& toCatch) {
         cerr << "Error during Xerces-c Initialization.\n"
              << "  Exception message:"
              << toCatch.getMessage() << endl;
         MC2_ASSERT(false);
      }

      // Create mapIndex object.
      const char* mapPath = "./";
      MapIndex mapIndex(mapPath);
      mapIndex.load();
   
      // Add data to index.
      addRegions(CL_arXMLFilesStr, mapIndex);
      if ( CL_addMapSupCoverage ){
         addMapSupCoverage(mapIndex, 
                           CL_crbXMLFilesStr, 
                           CL_mapSupNamesXMLFilesStr );
      }
      
      mc2log << info << "Saving index.db" << endl;
      mapIndex.save();

      // Shut down the XML system
      XMLPlatformUtils::Terminate();         

      exit(0);
   }

   else if (CL_updateCreationTimes){
      updateCreationTimes();
      exit(0);
   }
   else if (CL_createOverview){
      createOverview(CL_coXMLFilesStr);  
      exit(0);
   }
   else if ( (CL_printMidMifMapGfx != NULL) &&
             ((CL_startAtMap != 0) || (CL_endAtMap != MAX_UINT32)) ) {
      // print midmif map gfxdata using startAtMap and endAtMap
      
      // get map path from tail if maps are not in this directory
      const char* mapPath = "./";
      if (coh.getTailLength() == 1) {
         cout << "Setting map path to \"" << coh.getTail(0) << "\"" << endl;
         mapPath = coh.getTail(0);
      }
      
      // print mif header only once, i.e. for the first map
      bool printMifHeader = true;
      for (uint32 mapID = CL_startAtMap; mapID <= CL_endAtMap; mapID++) {
         GMSMap* curMap = 
               static_cast<GMSMap*>(GMSMap::createMap(mapID, mapPath));

         if (curMap != NULL) {
            cout << "Print map gfxdata for map " << mapID
                 << " to \"" << CL_printMidMifMapGfx << ".mid/mif\""
                 << endl;
            
            // Print gfxdata of the municipals in the map
            bool municipals = true;
            curMap->printMidMifMapGfxData(CL_printMidMifMapGfx,
                                          municipals, printMifHeader);
            // Print mif header only for the first map
            if (printMifHeader)
               printMifHeader = false;
         
            delete curMap;
         
         } else {
            exit(0);
         }
      }

      exit(0);
      
   }
   else if (CL_relabel[0] != MAX_UINT32){
         // Rename attributes
         int32 oldID = CL_relabel[0];
         int32 newID = CL_relabel[1];
         
         mc2log << info << "Calling changeMapID(" << oldID << ", "
                << newID << ")" << endl;
         if (changeMapID(oldID, newID)){
            mc2log << info << "ID changed to " << newID << endl;
         }
         else {
            mc2log << error << "Error changing ID to " << newID << endl;
            exit(1);
         }
         exit(0);
   }
   

   

   SysUtility::setPriority( 10 );
   
   // ACTION!
   //---------------------------------------------------------------------
   // Process the files given in tail!
   //-----------------------------------------------------
   ZipSideFile* zipSideFile = NULL;
   for (uint16 pos = 0; pos < coh.getTailLength(); pos++) {
      MC2String fileName = MC2String(coh.getTail(pos));
      
      readFile_t fileType;
      if (!fileOk(fileName, fileType)) {
         returnCode++;
         continue;
      }

      if (fileType == gdf 
            ){
         cerr << "Cannot process gdf file, need more options" << endl;
         returnCode++;
         continue;
      }

      MC2String mapName = fileName;

      //
      // Do the processing
      //---------------------------------------
      GMSMap* theMap = NULL;
      
      if( ( fileType != mcm) ){



         // Create a new map.
         bool loadMap = false;
         uint32 mapID = CL_saveAsID;
         const char* mapPath = "./";
         theMap = static_cast<GMSMap*>
            ( GMSMap::createMap( mapID, mapPath, loadMap ) );
         theMap->getItemNames()->addString("MISSING");
         theMap->createOldBoundrySegmentsVector();
         theMap->setDrivingSide(!CL_driveOnLeftSide);
         theMap->setCountryCode(countryCode);
      }
      switch(fileType) {
         
         case gdf:
            {
               // Process GDF-file - empty for fileType=gdf
                
            }
            break;
         case mcm:
            {
               // printMidMif, get the map from mapName
               if (CL_printMidMif != NULL) {

                  theMap = 
                     static_cast<GMSMap*>(GMSMap::createMap(mapName.c_str()));
                  mc2log << info << "Print mid mif for map " 
                         << theMap->getMapID() << " " << mapName 
                         << " to " << CL_printMidMif << ".mid/mif" << endl;
                  ItemTypes::itemType type = 
                     ItemTypes::getItemTypeFromString(CL_printMidMif);
                  if ( (int) type > 
                       ( (int) ItemTypes::numberOfItemTypes -1) ) {
                     mc2log << fatal << "ItemType could not "
                            << "be extracted from the filename, \"" 
                            << CL_printMidMif << "\"" << endl;
                     exit(1);
                  } else {
                     mc2dbg1 << "ItemType is " << (int) type << endl;
                  }
                  
                  // Print mif header only for the first map given in tail
                  bool printMifHeader = false;
                  if (pos == 0)
                     printMifHeader = true;
                  theMap->printMidMif(CL_printMidMif, type, printMifHeader);
               }                  
               else if ( CL_printHeader ){
                  mc2log << info << "Will print map header" << endl;
                  OldGenericMapHeader* mapHeader = 
                     OldGenericMapHeader::createMap( mapName.c_str() );
                  if (mapHeader == NULL){
                     mc2log << error << "Could not open map: " << mapName
                            << endl;
                     exit(1);
                  }
                  vector<MC2String> toPrint = 
                     mapHeader->getHeaderAsStrings();
                  for ( uint32 i=0; i<toPrint.size(); i++ ){
                     mc2log << info << "   " << toPrint[i] << endl;
                  }
               }
               else if (CL_printMidMifMapGfx != NULL) {

                  theMap = 
                     static_cast<GMSMap*>(GMSMap::createMap(mapName.c_str()));
                  cout << "Print map gfxdata for map " << theMap->getMapID()
                       << " to \"" << CL_printMidMifMapGfx << ".mid/mif\""
                       << endl;
                  
                  // Print gfxdata of the municipals in the map
                  bool municipals = true;
                  
                  // Print mif header only for the first map given in tail
                  bool printMifHeader = false;
                  if (pos == 0)
                     printMifHeader = true;

                  theMap->printMidMifMapGfxData(CL_printMidMifMapGfx,
                                                municipals, printMifHeader);
                  
               }
               else if ( CL_gfxFilteringForCoPols ) {
                  mc2dbg << "createGfxFilteringForCountryPolygons: for map" 
                         << " in tail " << mapName << endl;
                  GMSCountryOverviewMap* coMap = 
                     dynamic_cast<GMSCountryOverviewMap*>
                     (GMSMap::createMap(mapName.c_str()));
                  if (coMap != NULL) {
                     if ( createGfxFilteringForOneCountryPolygon(coMap) ) {
                        coMap->save();
                     }
                  } else {
                     mc2dbg << " map in tail is no co-map?" << endl;
                  }
                  delete coMap;
               }
               else if ( CL_createBorderItems ) {
                  mc2dbg << "createBorderItemsInCoMaps: for map" 
                         << " in tail " << mapName << endl;
                  GMSCountryOverviewMap* coMap = 
                     dynamic_cast<GMSCountryOverviewMap*>
                     (GMSMap::createMap(mapName.c_str()));
                  if (coMap != NULL) {
                     if ( createBorderItemsInOneCoMap(coMap) > 0 ) {
                        coMap->save();
                     }
                  } else {
                     mc2dbg << " map in tail is no co-map?" << endl;
                  }
                  delete coMap;
               }
         
               
               // Only process the files if they were not 
               // already processed before when creating the countrymap.
               else if (! CL_createCountryMap) {
                  // This map is already in mc2 format, create with
                  // correct type
                  MC2String mapFileName = STLStringUtility::basename(mapName);
                  MC2String mapDirName = STLStringUtility::dirname(mapName);
                  mapDirName += "/";
                  uint32 mapID=
                     (uint32)strtoul(mapFileName.c_str(), (char**)NULL, 16);
                  mc2log << info << "\nAha, our own format, " << mapName 
                         << ", extractedID=" << mapID << endl;
                  OldGenericMap* theGenericMap = 
                     GMSMap::createMap(mapID,mapDirName.c_str());
                  theMap = static_cast<GMSMap*>(theGenericMap);
                  if(theMap == NULL){
                     cerr << "Bad mcm file" << endl;
                     MC2_ASSERT(false);
                  }

                  bool saveTheMap = false;
                  
                  if ( CL_regenerateTurnDescriptions ) {
                     mc2log << info << "Generating turns for " << mapID
                            << endl;
                     if( ! theMap->initTurnDescriptions() ) {
                        cerr << "fail" << endl;
                        returnCode++;
                        saveTheMap = false;
                     } else {
                        saveTheMap = true;
                     }
                  } 
                  if (CL_generateStreetsFromStreetSegments){
                     mc2log << info << "Generating streets for " << mapID
                            << endl;
                     theMap->generateStreetsFromStreetSegments();
                     //curMap->updateMunicipalArray();
                     theMap->setAllItemLocation(true);
                     saveTheMap = true;
                  }

                  if (CL_mixedPostProcessing){
                     mc2log << info << "Starts mixed processing for: "
                            << mapID << endl;
                     mixedPostProcessing(theMap);
                     saveTheMap = true;
                  }
                  
                  if (CL_addSectionedZipPOIs != NULL) {
                     mc2log << info << "Starts adding zip codes from "
                            << "zip POI files in :"
                            << CL_addSectionedZipPOIs << " to map: "
                            << hex  << "0x" 
                            << mapID << dec << endl;
                     if ( zipSideFile == NULL ){
                        // Create and load the side file object.
                        zipSideFile = new ZipSideFile();

                     }
                     MC2String sectionedZipsPath = CL_addSectionedZipPOIs;
                     if ( theMap->addZipsFromZipPOIFile( *zipSideFile, 
                                                         sectionedZipsPath )){
                        saveTheMap = true;
                     }
                     else {
                        mc2log << error << "addZipsFromZipPOIFile failed,"
                               << " exits!" << endl;
                        exit(1);
                     }
                  }


                  if (CL_addZipToCountryMap != NULL) {
                     GMSCountryOverviewMap* currentCountryOverviewMap = 
                        dynamic_cast<GMSCountryOverviewMap*>
                                    (GMSMap::createMap(CL_addZipToCountryMap));
                     if (currentCountryOverviewMap != NULL) {
                        OldOverviewMap* overviewMap =
                           dynamic_cast<OldOverviewMap*>(theGenericMap);
                        if (overviewMap != NULL) {
                           mc2log << info << "Adding zips to country map from " 
                                  << overviewMap->getMapID() << endl;
                           uint32 r = currentCountryOverviewMap
                              ->addZipCodeItems(overviewMap);
                           mc2dbg << "Added " << r << " zip codes to "
                                  << "country overview map" << endl;
                           currentCountryOverviewMap->save();
                        } else {
                           mc2log << fatal << "No overview map when trying"
                                  << " to add items to country map!" << endl;
                        }
                     } else {
                        mc2log << fatal << "No country map to add zips to!"
                               << endl;
                     }
                  }
                  
                  // Multi mcm map file processing.
                  if ( CL_tool != NULL ){
                     saveTheMap = GMSTool::processMap(CL_tool, theMap);
                  }
                  
                  if ( saveTheMap ) {
                     theMap->save();
                  }
                  
               }
            }
            break;
         case xml:
            {
               cout << "Found xml file in tail. Nothing to do." << endl; 
            } break;
            
         default:
            cerr << "GenerateMapServer internal error 1" << endl;
            exit(1);
      }

      // Do misc thing with the mcm map loaded from tail
      // The map "theMap" was created in the switch ("Aha, our own format")
      
      if (CL_setLocations && (CL_createItemsFromMidMif == NULL)) {
         if(CL_verboseLevel > 1) {
            cout << "Setting location" << endl;
         }
         // Update municipal array in case of any new bua/mun being added.
         theMap->updateMunicipalArray();
         theMap->setAllItemLocation(true);

         if (CL_verboseLevel > 1) {
            cout << "Saving map" << endl;
         }
         theMap->save();
      }

      if(CL_saveAsID != MAX_UINT32) {
         if(CL_verboseLevel > 1) {
            cout << "Saving map as ID " << CL_saveAsID << endl;
         }
         theMap->setMapID(CL_saveAsID);
         theMap->setFilename("./");
         theMap->save();
      }

      if ( CL_filterCoordinates ) {
         mc2dbg8 << "To filter coordinates in map " 
                 << theMap->getMapID() << endl;
         
         if ( CL_filterLevels ) {
            // An alternative way of coordinate filtering.
            // Might be better than the normal method, for future use
            // for better map display of items
            GMSUtility::filterCoordinateLevels( theMap );

         } else {
            // The normal coordinate filtering
            GMSUtility::filterCoordinates( theMap );
         
         }
         
         // Re-build underview map gfxData in case filtering changed geometry.
         // Use convex hull of street segments and points of interest.
         if ( MapBits::isUnderviewMap(theMap->getMapID()) ) {
            mc2dbg << "To re-build map gfx data for map "
                   << theMap->getMapID() << endl;
            theMap->setMapGfxDataConvexHull();
            mc2dbg << " Done re-built" << endl;
         }
         
         theMap->save();
      }

      if ( CL_filterCountryPolygon ) {
         
         // extract break points and filter theMap
         OldGenericMap* map = dynamic_cast<OldGenericMap*>(theMap);
         GMSCountryOverviewMap* countryMap = 
            dynamic_cast<GMSCountryOverviewMap*>(map);
         mc2log << info << "filterCountryPolygon map " << theMap->getMapID()
                << endl;
         
         bool filtered = false;
         // Copy filtering from another mcm co map ?
         if ( CL_copyFilteringFromMap ) {
            mc2log << info << "Copy filtering from " << CL_copyFilteringFromMap
                   << endl;
            if ( !  StringUtility::endsWithStr( 
                     CL_copyFilteringFromMap, ".mcm" ) ) {
               MC2_ASSERT(false);
            }
            GMSCountryOverviewMap* filterMap = 
               dynamic_cast<GMSCountryOverviewMap*> 
               (GMSMap::createMap( CL_copyFilteringFromMap ));
            if ( filterMap != NULL ){
               if ( filterMap->getCountryCode() ==
                           countryMap->getCountryCode() )
               { 
                  if ( filterMap->mapGfxDataIsFiltered() ) {
                     if ( countryMap->copyCoPolFilteringFromMap(
                              filterMap, CL_countryPolygonBreakPoints) ) {
                        mc2dbg1 << "Copied country polygon filtering from "
                                << CL_copyFilteringFromMap << " to map "
                                << countryMap->getMapID() << endl;
                        filtered = true;
                     } else {
                        mc2log << error << " while copying country polygon "
                               << "filtering from " << CL_copyFilteringFromMap
                               << " to map " << countryMap->getMapID()
                               << endl;
                     }
                  } else {
                     // The found map is not filtered, filter this co map
                     mc2dbg1 << "The found map has no coordinate filtering "
                             << "in co pol - will filter here!" << endl;
                     // coordinate filtering
                     filtered = filterCoPolCoordinateLevels(
                           countryMap, CL_countryPolygonBreakPoints );
                     
                     //// gfx filtering
                     //countryMap->makeSimplifiedCountryGfx();
                  }
               }
               else {
                  mc2log << error << "GenerateMapServer::" 
                         << "filterCountryPolygon. Not same country! "
                         << "Exits!" << endl;
                  MC2_ASSERT(false);
               }
            }
            else {
               mc2log << error << "GenerateMapServer::" 
                      << "filterCountryPolygon. Error opeing country"
                      << "map to fetch filtering from. Exits!" << endl;
               MC2_ASSERT(false);
            }
            delete filterMap;
         }
         // else no co map to copy from
         else {
            filtered = filterCoPolCoordinateLevels(
                  countryMap, CL_countryPolygonBreakPoints );
         }
         if ( filtered ) {
            countryMap->save();
         }
         
      }
         
      delete theMap;
   }
   delete zipSideFile;

   // The files in tail processed.
   // Now, look for additional things.
   while(true) {
      
      if( CL_regenerateTurnDescriptions &&
               ( coh.getTailLength() == 0 ) &&
               ( CL_createItemsFromMidMif == NULL) ) {
         //
         // Loop over all the maps in current directory
         // and regenerate turndescriptions.
         //---------------------------------------------------------------
         cout << "Regenerate Turndescriptions" << endl;
         if(CL_verboseLevel > 1) {
            cout << "Regenerating turn descriptions" << endl;
         }

         const char* mapPath = "./";
         for(uint32 mapID = 0; ; mapID++) {
            GMSMap* curMap = static_cast<GMSMap*>
                                 (GMSMap::createMap(mapID, mapPath));
            if(curMap == NULL) {
               break;
            } else {
               
               if(CL_verboseLevel > 2) {
                  cout << "\t" << setw(6) << mapID << endl;
               }
               
               if(!curMap->initTurnDescriptions()) {
                  cerr << "fail" << endl;
                  returnCode++;
               }
               curMap->save(); 
            }
            delete curMap;
         }
         CL_regenerateTurnDescriptions = false;
      }
      
      else if (CL_createItemsFromMidMif != NULL){
         //
         // Loop over all maps in current directory to
         // create and add items
         // Use the startAtMap and endAtMap if provided.
         cout << "Create items from mid mif, file: " // don't change these
              << CL_createItemsFromMidMif << endl;   // lines! they are used
                                                     // in error detection
         MC2String fileName = CL_createItemsFromMidMif;
         MC2String midName = (fileName + ".mid");
         MC2String mifName = (fileName + ".mif");
         ifstream test1(midName.c_str());
         ifstream test2(mifName.c_str());
         if (test1 && test2) {
            // Ok, item file exists
            test1.close();
            test2.close();

            if(CL_verboseLevel > 1) {
               cout << "Creating items" << endl;
            }

            uint32 totNbrCreated = 0;
            uint32 nbrItemsInFile = 0;
            bool allMidMifItemsCreated = false;
            const char* mapPath = "./";
            for (uint32 mapID = CL_startAtMap; mapID <= CL_endAtMap; mapID++) {
               if ( allMidMifItemsCreated ) {
                  mc2log << info << "All " << nbrItemsInFile 
                         << " midmif items created,"
                         << " break before loading map " << mapID << endl;
                  break;
               }
               GMSMap* curMap = static_cast<GMSMap*>
                                    (GMSMap::createMap(mapID, mapPath));
               if (curMap == NULL) {
                  if ( !allMidMifItemsCreated ) {
                     if ( ! CL_loopAllMaps ) {
                        // Do not change this debug!
                        // It is used in error detection
                        mc2log << warn << "Only created and added "
                               << totNbrCreated << " of " << nbrItemsInFile
                               << " midmif items to the looped mcm maps"
                               << endl;
                     } else {
                        // Do not change this debug!
                        // It is used in error detection
                        mc2log << "Created and added "
                               << totNbrCreated << " of " << nbrItemsInFile
                               << " midmif items to the looped mcm maps"
                               << endl;
                     }
                  }
                  break;
               } else {
                  if (CL_verboseLevel > 2) {
                     cout << "\t" << setw(6) << mapID << endl;
                  }
                  // combine with CL_setLocations,
                  // CL_regenerateTurnDescriptions and
                  // CL_updateNodeLevelsInMap
                  GMSMidMifHandler mmh(curMap);
                  int nbrCreated =
                     mmh.createItemsFromMidMif(
                           CL_createItemsFromMidMif, 
                           nbrItemsInFile, //outparam
                           true, // addToExistingMap
                           CL_setLocations,
                           CL_regenerateTurnDescriptions,
                           CL_updateNodeLevelsInMap,
                           CL_useCoordToFindCorrectMap,
                           CL_addAllMidMifItems,
                           CL_tryToBuildMapGfxFromMunicipals);
                  if (nbrCreated == -1) {
                     mc2log << fatal << "Failed to create items in map "
                            << curMap->getMapID() << endl;
                     MC2_ASSERT(false);
                  } else if (nbrCreated > 0) {
                     totNbrCreated += nbrCreated;
                     mc2log << info << "Created " << nbrCreated << " items "
                            << "(now totally " << totNbrCreated <<  " of "
                            << nbrItemsInFile
                            << ") saving map " << curMap->getMapID() << endl;
                     if ( ! CL_loopAllMaps && 
                          (totNbrCreated == nbrItemsInFile) ) {
                        allMidMifItemsCreated = true;
                     }
                     curMap->save();
                  } else {
                     mc2dbg1 << "No items created in map " 
                             << curMap->getMapID() 
                             << " (totally " << totNbrCreated <<  " of "
                             << nbrItemsInFile << ")" << endl;
                  }
               }
               delete curMap;
            }
         } else {
            cerr << "Cannot open item file "
                 << CL_createItemsFromMidMif << endl;
            returnCode++;
         }

         CL_createItemsFromMidMif = NULL;
         CL_setLocations = false;
         CL_regenerateTurnDescriptions = false;
         CL_updateNodeLevelsInMap = false;
         CL_addAllMidMifItems = false;
         CL_tryToBuildMapGfxFromMunicipals = false;
         
      }
      else if (CL_removeDupItemsFromMidMif != NULL) {
         
         removeDupItemsFromMidMif();
         CL_removeDupItemsFromMidMif = NULL;
      
      } else if (CL_initMapFromMidMif != NULL) {
         
         mc2dbg << "Init map for creation from mid/mif with file "
                << CL_initMapFromMidMif << endl;
         MC2String fileName = CL_initMapFromMidMif;
         MC2String midName = (fileName + ".mid");
         MC2String mifName = (fileName + ".mif");
         MC2String mapmifName = (fileName + "map.mif");
         ifstream test1(midName.c_str());
         ifstream test2(mifName.c_str());
         ifstream test3(mapmifName.c_str());
         if (test1 && test2 && test3) {
            //Ok, file exists
            test1.close();
            test2.close();
            test3.close();


            // Create a new map and set misc map data
            const char* mapPath = "./";
            bool loadMap = false;
            GMSMap* newMap = static_cast<GMSMap*>
               ( GMSMap::createMap( CL_saveAsID, mapPath, loadMap ) );
            newMap->getItemNames()->addString("MISSING");
            mc2dbg << "Setting country code " << int(countryCode) << endl;
            newMap->setCountryCode(countryCode);
            if (CL_copyrightString != NULL) {
               mc2dbg << "Setting copyright string '" << CL_copyrightString 
                      << "'" << endl;
               newMap->setCopyrightString(CL_copyrightString);
            }
            if ( CL_name != NULL ){
               newMap->setMapName( CL_name );
               mc2dbg << "Setting map name '" << CL_name << "'" << endl;
            }
            if ( CL_maporigin != NULL ) {
               newMap->setMapOrigin (CL_maporigin);
               mc2dbg << "Setting map origin '" << CL_maporigin << "'" << endl;
            }
            newMap->setDrivingSide(!CL_driveOnLeftSide);
            
            
            // Create the municipal items
            GMSMidMifHandler mmh(newMap);
            uint32 nbrItemsInFile = 0;
            int nbrCreated =
               mmh.createItemsFromMidMif(CL_initMapFromMidMif,
                                         nbrItemsInFile, // out param
                                         false); // addToExistingMap
            if (nbrCreated == -1) {
               mc2log << fatal << "Failed to create items in map "
                      << newMap->getMapID() << endl;
               MC2_ASSERT(false);
            } else {
               mc2dbg1 << "Map initiated with " << nbrCreated
                       << " municipals (totally " << nbrItemsInFile
                       << " mun in file)" << endl;
            }
            newMap->compactMunicipals();

            //open files for creating the mapGfx
            char* mapmifFileName = new char[strlen(CL_initMapFromMidMif) + 10];
            strcpy(mapmifFileName, CL_initMapFromMidMif);
            strcat(mapmifFileName, "map.mif");
            ifstream mapmiffile(mapmifFileName);
            delete mapmifFileName;
            
            //create and set the map gfxData
            mc2dbg << "Creating map gfxData from " << mapmifName << endl;
            GMSGfxData* mapGfx = 
               GMSGfxData::createNewGfxData(NULL, false);
            mapGfx->createFromMif(mapmiffile);
            mc2dbg << "Setting map gfxData" << endl;
            newMap->setGfxData( mapGfx );
            newMap->save();
            delete newMap;
         } else {
            mc2log << error << here << "Cannot open file "
                 << CL_initMapFromMidMif << " .mid .mif map.mif" << endl;
            returnCode++;
         }
         
         CL_initMapFromMidMif = NULL;

      
      } else if ( CL_generateStreetsFromStreetSegments &&
                  coh.getTailLength() == 0 ){
         cout << " Generate streets from street segments" << endl;
         const char* mapPath = "./";
         for (uint32 mapID = 0; ; mapID++) {
            GMSMap* curMap = static_cast<GMSMap*>
                                (GMSMap::createMap(mapID, mapPath));
            if (curMap == NULL) {
               break;
            } else {
               if (CL_verboseLevel > 2) {
                  cout << "\t" << setw(6) << mapID << endl;
               }
               curMap->generateStreetsFromStreetSegments();
               curMap->updateMunicipalArray();
               curMap->setAllItemLocation(true);
               curMap->save();
            }
            delete curMap;
         }
         CL_generateStreetsFromStreetSegments = false;

      } else if ( CL_createBorderBoundrySegments ) {
      
         mc2dbg1 << "Create midmif boundry segments" << endl;
         GMSUtility::createBorderBoundrySegments( "./", CL_startAtMap );
         CL_createBorderBoundrySegments = false;

      } else {
         mc2log << info << "Done!" << endl;
         break;
      }
   }

   if(returnCode > 0)
      cerr << "There were " << returnCode << " errors." << endl;

   return returnCode;
}

// Hack to make a string out of a 32-bit unsigned integer.
// Note: The implementation could be dangerous on platforms
//       with very large integers. On IA-32 we really don't
//       need more than 12 elements in the char tmp array, but
//       it's better to be sure...
void uint2str(uint32 ui, MC2String& s)
{
   s.erase();
   char tmp[24];
   tmp[23] = 0;
   int pos = 23;
   
   while(ui > 0) {
      tmp[--pos] = ('0' + (char)(ui % 10));
      ui /= 10;
   }

   s = &tmp[pos];
}

void initCommandline(CommandlineOptionHandler& coh)
{
   MC2String strmaxuint32;
   uint2str(MAX_UINT32, strmaxuint32);
   MC2String strzero;
   uint2str(0, strmaxuint32);


   coh.setSummary("Generate maps from raw datafiles");
   coh.setTailHelp("[file...]");

   //
   // VERBOSE LEVEL ( -V / --verbose )
   //---------------------------------------------------------------------
   coh.addOption("-V", "--verbose",
                 CommandlineOptionHandler::uint32Val,
                 1, &CL_verboseLevel, "0",
                 "Set verbose level:\n"
                 "0:\t  (default) Output nothing\n"
                 "1:\t  \n"
                 "2:\t  \n"
                 "3:\t  Fill terminal window with lots of prints");
   
   //
   // SAVE THE MAP AS GIVEN ID ( -N / --saveas )
   //---------------------------------------------------------------------
   coh.addOption("-N", "--saveas",
                 CommandlineOptionHandler::uint32Val,
                 1, &CL_saveAsID, strmaxuint32.c_str(),
                 "The ID that will be assigned to the map.");

   //
   // DRIVE ON LEFT SIDE ( -L / --driveonleftside )
   //---------------------------------------------------------------------
   coh.addOption("-L", "--driveonleftside",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_driveOnLeftSide, "F",
                 "Drive on left side of the road");

   //
   // COUNTRY ( -C / --country )
   //---------------------------------------------------------------------
   coh.addOption("-C", "--country",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_country, "sweden",
                 "The gmsName of the country of the map to be generated\n"
                 "Supported countries are e.g.:\n"
                 "sweden\n"
                 "norway\n"
                 "denmark\n"
                 "finland\n"
                 "germany\n"
                 "england\n"  // This is UK, spelled out as england due to 
                              // old mistakes. Please leave it like this.
                 "belgium\n"
                 "netherlands\n"
                 "luxembourg\n"
                 "usa\n"
                 "switzerland\n"
                 "austria\n"
                 "france\n"
                 "spain\n"
                 "andorra\n"
                 "liechtenstein\n"
                 "italy\n"
                 "monaco\n"
                 "ireland\n"
                 "portugal\n"
                 "canada\n"
                 "hungary\n"
                 "czech_republic\n"
                 "poland\n"
                 "greece\n"
                 "israel\n"
                 "etc...\n"
                 "Please look in the code to find out how to specify the "
                 "countries. See MapGenUtil::getCountryCodeFromGmsName "
                 "where the gmsName is translated to "
                 "StringTable::countryCode, which defines the country "
                 "in the map generation and the MC2 server."
                 );

   
   //
   // NAME ( -a / --name )
   //---------------------------------------------------------------------
   coh.addOption("-a", "--name",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_name, "",
                 "The name of the mcm map to create\n");

   //
   // MAP ORIGIN ( -S / --maporigin )
   //---------------------------------------------------------------------
   coh.addOption("-S", "--maporigin",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_maporigin, "",
                 "The origin of the map, containing supplier and"
                 " release of the map files used for building the map."
                 " Example TeleAtlas_2009_09" );

   //
   // Set the copy right string in the map (map header)
   //---------------------------------------------------------------------
   coh.addOption("", "--copyrightString",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_copyrightString, "",
                 "Set the copy right string in the map to a new string. "
                 "Used when creating mcm maps from scratch.\n"
                 "NB: This copyright from map header is not used when "
                 "running maps on the server, if we used the copyright "
                 "map supplier boxes xml file in the map generation.");

   
   //
   // Init one map for createItemsFromMidMif
   //---------------------------------------------------------------------
   coh.addOption("-g","--initMapFromMidMid",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_initMapFromMidMif, "",
                 "Init one map for creation from mid/mif. It is initialised "
                 "from the file with municipals that will be in the map. "
                 "Give the filename without extension and combine with "
                 "the --saveas option. Note! The item must be in "
                 "the file name (\"municipalItem\").");
   
   //
   // Create items from midmif
   //---------------------------------------------------------------------
   coh.addOption("-r", "--createItemsFromMidMif",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_createItemsFromMidMif, "",
                 "Create items from a mid and mif file. Give the "
                 "filename without extensions as value to this option.\n"
                 "Loops all "
                 "underview maps in the current dir and adds the items.\n"
                 "Note1: The item must be in the file name to know what "
                 "kind of item to create. If creating street segments, the "
                 "file name can be e.g. streetSegmentItem_newRbt.mif \n"
                 "Combine with options:\n"
                 " --setlocations to set location of added items\n"
                 " -t if turn descriptions should be generated for new "
                 "    street segments\n"
                 " --updateNodeLevels if node levels must be updated\n"
                 " --useCoordToFindCorrectMap if using a map ssi coordinate "
                 "   to decide correct map for each item.\n"
                 " --addAllMidMifItems if all items should be added, not "
                 " checking settlement ids, geometry or map ssi "
                 " coordinate\n"
                 " --loopAllMaps if all mcm maps should be looped even "
                 "   if all items have been added to the maps. Use if "
                 "   adding to maps with no exact mapgfxdata and no "
                 "   mapssicoord or admin info given in the midmif file.\n"
                 " --startAtMap --endAtMap for controlling which mcm "
                 " map(s) in this dir to loop.");
   
   //
   // If a coordinate is given to decide the correct map.
   coh.addOption("", "--useCoordToFindCorrectMap",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_useCoordToFindCorrectMap, "F",
                 "Used with -r/--createItemsFromMidMif. There is an mc2 "
                 "map ssi ccordinate is given at the end of each mid row "
                 "to decide which map is correct (\"midrow,mc2lat,mc2lon\"). "
                 "The coordinate should be within 2 meters from any "
                 "street segment item in the map.\n"
                 "This is used when the map gfx is the convex hull and "
                 "not the exact map boundry.");

   //
   // Whether to update node levels for new street segments or not
   coh.addOption("", "--updateNodeLevels",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_updateNodeLevelsInMap, "F",
                 "Used with -r/--createItemsFromMidMif."
                 "If mid mif supplier had level on street segments, the "
                 "node levels need to be updated in order not to remove "
                 "connections between nodes with different levels in "
                 "the post processing.");

   //
   // Whether to force looping all maps for addition of midmif items
   coh.addOption("", "--loopAllMaps",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_loopAllMaps, "F",
                 "Used with -r/--createItemsFromMidMif. "
                 "If all maps should be looped, not aborting when nbr of "
                 "items added to the maps exceeds the nbr of items in the "
                 "midmif file. "
                 "To be used when the map gfx data is the convex hull or "
                 "bbox and you do not have mapssicoord etc to decide which "
                 "is the correct map. "
                 "Need to run the method --removeDupItemsFromMidMif "
                 "afterwards, removing the midmif items ending up in wrong "
                 "map (within convexHull/bbox, but outside true map gfx)");
   
   //
   // Whether to force addition of all midmif items to the mcm map or not
   coh.addOption("", "--addAllMidMifItems",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_addAllMidMifItems, "F",
                 "Used with -r/--createItemsFromMidMif. "
                 "If all items in the midmif file should be added to the "
                 "mcm map, not checking if it fits with settlement ids, "
                 "map ssi coordinates or geometry (inside map gfx data).\n"
                 "The option is typically used if there is only one map "
                 "in the country map generation, so you know that all "
                 "items should be added to the one map.");
   //
   // Whether to try to build a good map gfx for adding the midmif items
   coh.addOption("", "--tryToBuildMapGfxFromMunicipals",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_tryToBuildMapGfxFromMunicipals, "F",
                 "Used with -r/--createItemsFromMidMif. "
                 "If you want to try to build a good mapgfx merging "
                 "the municipals in the map, so you can add the midmif "
                 "items to the correct map based on geometry "
                 "(inside map gfx).\n"
                 "Use if you do not have map ssi coord, admin info or "
                 "anything else to use for deciding correct map, when "
                 "the map gfx is bbox/conv hull");
   //
   // Remove midmif iteams that ended up in the wrong mcm map
   //---------------------------------------------------------------------
   coh.addOption("","--removeDupItemsFromMidMif",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_removeDupItemsFromMidMif, "",
                 "Remove the midmif items that ended up in wrong "
                 "map (within convexHull/bbox, but outside true map gfx) "
                 "when running -r/--createItemsFromMidMif.\n"
                 "Give the midmif file as value to this option.");
   //
   // Create boundry segments from geographical location (e.g. midmif maps)
   //---------------------------------------------------------------------
   coh.addOption("", "--createBorderBoundrySegments",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_createBorderBoundrySegments, "F",
                 "For all maps in this directory, the boundry segments "
                 "vector is created from the geographical location of the "
                 "street segments. The map must have a true mapGfxData, "
                 "it must not be a bounding box or a convex hull.\n"
                 "Option is used for maps generated from midmif.\n");

   //
   // Add data from external maps, so called extra item maps
   //---------------------------------------------------------------------
   coh.addOption("-u", "--addItemsFromExtraItemMaps",
                 CommandlineOptionHandler::presentVal,
                 1, &CL_addItemsFromExtraItemMaps, "F",
                 "Add segments and other items (such as street segments, "
                 "ferries forests and lakes) from extraitem maps.\n"
                 "Extraitem maps are external maps, not part of the original "
                 "country map generation, but maps with some extra items "
                 "that need to the added to the normal maps in order for "
                 "e.g. external connections between countries to work. It "
                 "might contain e.g. items such as street segments for "
                 "bridges/tunnels and ferry lines lying in outerworld = "
                 "ocean areas on international water.\n"
                 "Extra items are assumed to be found in maps with "
                 "map id 0x2001 and up, located in the map generation "
                 "directory. All normal mcm maps, "
                 "starting from id 0 and until no more maps are found will be "
                 "looped to add items from the extra item maps.\n\n"
                 
                 "Addition from extra item maps should be done early in "
                 "the map generation and considered to be part of the raw "
                 "map data from the map supplier. Must thus be executed "
                 "before any kind of post processing.");


   //
   // Mixed post processing.
   //---------------------------------------------------------------------
   coh.addOption("", "--mixed-post-proc",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_mixedPostProcessing,"F",
                 "The first step of post processing of maps generated from "
                 "the map supplier map data.\n"
                 "Takes care of some special tasks such as: "
                 "setting native langugare, remove unwanted no-name buas "
                 "and ocean waters, verify that search areas have a valid "
                 "name, merging close items and administrative areas, set "
                 "location for all items with invalid locations, eliminate "
                 "holes and self-touches from airports, set area feature "
                 "draw display class.\n"
                 "Must be followed by generate streets, generate turn "
                 "descriptions, set location and finish index areas."
                 );
   
   //
   // Add addres point names
   //---------------------------------------------------------------------
   coh.addOption("", "--addAddrPointNames",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_addrPointFile, "",
                 "Add address point names to SSIs from the address point "
                 "file given as value to this option. "
                 "All underview maps in the map generation directory will "
                 "be looped and have names added to the street segments "
                 "in the map. The address point name will be added as "
                 "alternative name in english (hardcoded) to the closest "
                 "street segment. "
                 "The maximum distance from the address point to the street "
                 "segment is hardcoded (500 meters).\n"
                 "The format of data rows in the address point file is:\n"
                 "addressPointID;mc2-lat;mc2-lon;addressPointName\n"
                 "example:\n"
                 "L0000069051;207973030;937106196;APHB Colony Tarnaka\n"
                 "The ID must be unique in the file and char encoding UTF-8."
                 );

   //
   // Add zip codes from zip POI file
   //---------------------------------------------------------------------
   coh.addOption("", "--addSectionedZipPOIs",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_addSectionedZipPOIs, "",
                 "Adds zip codes to the mcm maps in tail from zip POI files "
                 "in the directory given as value to this option."
                 "The format of data rows in the zip POI file is:\n"
                 "||zipcode|mc2-lat|mc2-lon\n"
                 "example:\n"
                 "||8360223|-706558699|-334613299\n"
                 "(the first two columns are not used)"
                 );


   //
   // Generate streets from street segments
   //---------------------------------------------------------------------
   coh.addOption("-J", "--generateStreetsFromStreetSegments",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_generateStreetsFromStreetSegments,"F",
                 "Generating streets from street segments. This should"
                 "be done after the maps have been saved to disk," 
                 "in case name corrections need to be done before");

   // 
   // REGENERATE TURN DESCRIPTIONS ( -t / --regturn )
   //---------------------------------------------------------------------
   coh.addOption("-t", "--regturn",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_regenerateTurnDescriptions, "F",
                 "Regenerate turndescriptions. For all maps given in tail "
                 "or all maps in the current directory.");

   //
   // SET LOCATION FOR ALL ITEMS ( -l / --setlocations )
   //---------------------------------------------------------------------
   coh.addOption("-l", "--setlocations",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_setLocations, "F",
                 "Set the location for all items");


   //
   // Finish up index areas
   //---------------------------------------------------------------------
   coh.addOption("", "--finishIndexAreas",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_finishIndexAreas,"F",
                 "Finish index areas, it will remove geometry from "
                 "index areas and customize the hierarchies.\n"
                 "Must be done as a last step of map generation, e.g. it "
                 "will not work to modify locations after this step.\n"
                 "If there are no index areas in the maps of this map "
                 "generation nothign will happen.");


   //
   // SET THE EXTERNAL CONNECTIONS
   //---------------------------------------------------------------------
   coh.addOption("-e", "--extconnections",
                 CommandlineOptionHandler::presentVal,
                 1, &CL_extconnections, "F",
                 "Check and update the external connections.\n"
                 "Creates external connections between all maps present "
                 "in the map generation directory, within the intervall "
                 "of --startAtMap and --endAtMap. Default all maps, with no "
                 "specific startAt/endAt set.\n"
                 "It might be underview maps, first level overview maps or "
                 "second level overview maps");


   coh.addOption("", "--processNeighbours",
                 CommandlineOptionHandler::presentVal,
                 1, &CL_processNeighbours, "F",
                 "The external connection of the neighbours of the maps "
                 " given with --startAtMap and --endAtMap are also "
                 "processed. To be used when patching in a merge.");



   //
   // Create country overview maps
   //---------------------------------------------------------------------
   coh.addOption("-X", "--createcountrymaps",
                 CommandlineOptionHandler::presentVal,
                 1, &CL_createCountryMap, "F",
                 "Create country maps (id from 0x80000001 and higher) "
                 "In the tail, supply:\n"
                 "- XML file containing create_overviewmap tags in order "
                 "to specify which uderview maps that should belong to "
                 "which country map.\n"
                 "- Optional country overview maps for reuse of map gfx "
                 "data. This speeds up the generation process a lot.\n\n"

                 "There should be one mif-file containing the map gfx "
                 "data for each country map. These mif-files must be "
                 "present in ./ and be named [country_map_name].mif.\n\n "
                 
                 "Possible additional options:\n"
                 "--coPolBreakPoints\n");

   //
   // Filter country polygon, coordinate filtering ( --filterCountryPolygon )
   //---------------------------------------------------------------------
   coh.addOption("", "--filterCountryPolygon",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_filterCountryPolygon, "F",
                 "Filter country polygon, resulting in 16 filter levels "
                 "stored in coordinates. Combine with --coPolBreakPoints "
                 "to give file with break points.\n"
                 "If you have a co map you want to copy the filtering from "
                 "give option --copyFilteringFromMap\n"
                 "Don't forget proceed with "
                 "building gfx filtering --gfxFilteringOfCoPols! "
                 "Then possible for --createBorderItems.");
   
   coh.addOption("", "--coPolBreakPoints",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_countryPolygonBreakPoints, "",
                 "Text file with break point coordinates defining shared "
                 "lines between neighbouring country polygons. Coordinates "
                 "expressed in mc2 coordinates, use spaces to delimit fields\n"
                 "\"lat lon description\"\n"
                 "823915938 245139346 swe-fin-no\n"
                 "785357201 288128788 swe-fin-ocean");
   
   coh.addOption("", "--copyFilteringFromMap",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_copyFilteringFromMap, "",
                 "Help option to --filterCountryPolygon. The filtering will "
                 "be copied from this map, instead of built from scratch");
   //
   // Create gfx filtering of country polygons ( --gfxFilteringOfCoPols )
   //---------------------------------------------------------------------
   coh.addOption("", "--gfxFilteringOfCoPols",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_gfxFilteringForCoPols, "F",
                 "Loop all country overview maps and create the gfx "
                 "filtering of country polygons (coord idx, level 0-6). "
                 "The co maps should preferrably have country polygons "
                 "filtered (coordinate filtering), otherwise this process "
                 "is time consuming.");
   //
   // Create border items
   //---------------------------------------------------------------------
   coh.addOption("", "--createBorderItems",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_createBorderItems, "F",
                 "Create border items in all co maps in this directory, "
                 "or give specific co maps in this directory in tail. "
                 "The borderItems.txt and countryBorders.txt must exist!\n"
                 "Use togheter with --coPolBreakPoints.");

   //
   // Create overvies
   //---------------------------------------------------------------------
   coh.addOption("", "--createOverview",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_createOverview, "F",
                 "Create next not created overview map. Use the options:"
                 " --coXML xml files defining map hierarchy, both for "
                 "overviews and super overviews. Exits with 77 if no "
                 "overview map to create exists.");


   //
   // Add zip codes to co-maps from ov-maps
   //---------------------------------------------------------------------
   coh.addOption("", "--addziptocountrymaps",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_addZipToCountryMap, "",
                 "Add zip codes to excisting country maps (id from "
                 "0x80000001 and higher). Will use information from "
                 "a overviewmap");

   //
   // Handling index.db.
   //---------------------------------------------------------------------
   coh.addOption("", "--createNewIndex",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_createNewIndex, "F",
                 "Create a new index.db for existing maps, i.e. rebuilds "
                 "index.db from scratch. The data set in index.db is "
                 "fetched from the existing maps. Use the options:"
                 " --coXML and -cooXML to specify the xml files"
                 " to use when creating the index. Specify at least the"
                 " -coXML files. This option does not add regions or"
                 " update creation times.");

   coh.addOption("", "--coXML",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_coXMLFilesStr, "",
                 "Use this option together with --createNewIndex. "
                 "Specifies the create overview map xml files used for "
                 "creating overviews. If more than one, give a space-"
                 "separated list within quotes(\").");

   coh.addOption("", "--cooXML",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_cooXMLFilesStr, "",
                 "Use this option together with --createNewIndex. "
                 "Specifies the create overview map xml files used for "
                 "creating super overviews. If more than one, give a "
                 "space-separated list within quotes(\").");


   coh.addOption("", "--addRegionsToIndex",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_addRegionsToIndex, "F",
                 "Add regions to the index. Use the option: --arXML "
                 "to specify the xml files including the regions "
                 "to add.");

   coh.addOption("", "--arXML",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_arXMLFilesStr, "",
                 "Use this option together with --addRegionsToIndex. "
                 "Specifies the add region xml files telling what "
                 "regions to use, and the region_ids.xml file telling the "
                 "the IDs of the regions. Use a space separated list "
                 "within quotes(\").");

   coh.addOption("", "--addMapSupCoverage",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_addMapSupCoverage, "",
                 "Add geometrical representation of different map "
                 "suppliers coverage. To be used in the client, so the "
                 "coverage must have a very compact representation.\n"
                 "Use as the only option or together with "
                 "--addRegionsToIndex. Give file name of XML file to use "
                 "as argument.");

   coh.addOption("", "--crbXML",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_crbXMLFilesStr, "",
                 "Use this option together with --addMapSupCoverage. "
                 "Specifies the copyright map supplier boxes xml file "
                 "telling what areas are covered by which map suppliers.\n"
                 "If this is used in map generation, the copyright string "
                 "in the map file is never used.");

   coh.addOption("", "--mapSupNamesXML",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_mapSupNamesXMLFilesStr, "",
                 "Use this option together with --addMapSupCoverage. "
                 "Specifies the map supplier names xml files telling "
                 "the names of each map supplier in different languages. ");

   coh.addOption("-U", "--updateCreationTimes",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_updateCreationTimes, "F",
                 "Update creation stored in index.db and country maps.\n"
                 "Must always be run last afte rany kind of editing of any "
                 "of the maps in the map set.");

   //
   // Filter item polygons in map (  / --filterCoordinates )
   //---------------------------------------------------------------------
   coh.addOption("", "--filterCoordinates",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_filterCoordinates, "F",
                 "Filter coordinates in the mcm map(s) given in tail "
                 "(the maps must be in this directory). "
                 "For underview maps the map gfx data is re-built, "
                 "by calculating convex hull of street segments and pois.\n"
                 "Choose filter method using/not using --filterLevels "
                 "option.");
   //
   // Filter item polygons in map in different levels (  / --filterLevels )
   //---------------------------------------------------------------------
   coh.addOption("", "--filterLevels",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_filterLevels, "F",
                 "Filter items with new filtering method, resulting in 16 "
                 "filter levels. Combine with --filterCoordinates.\n"
                 "Warning:\n"
                 "This way of item coordinate filtering is not "
                 "much verified.");
   

   //
   // READ EXTRA DATA FILE ( -x / --extradata ) to apply map correction records
   //---------------------------------------------------------------------
   coh.addOption("-x", "--extradata",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_readExtraData, "",
                 "Read extra data (map correction records) from "
                 "the specified file, using startAtMap "
                 "and endAtMap (specified or default (0 resp MAX_UINT32)) "
                 "for looping mcm maps in this directory.\n"
                 "Combine this option with:\n"
                 " --checkextradata for an extra data validity check, check "
                 "if map correction records are valid for a new map release.\n"
                 " --extradataonmergedmaps for dynamic extra data, applying "
                 "map correction records to a merge." );

   //
   //---------------------------------------------------------------------
   coh.addOption("", "--usingDatabase",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_usingDatabase, "F",
                 "Use toghether with -x. Reults in setting the dynamic "
                 "extra data time if not already set. This option exists "
                 "because the time should not be set by a -x call from a "
                 "file used in for instance stitching a merge.");

   //
   // CHECK EXTRA DATA FILE (  / --checkextradata )
   //---------------------------------------------------------------------
   coh.addOption("", "--checkextradata",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_checkExtraData, "F",
                 "Run an extradata validity check. The extra data specified "
                 "with -x /--extradata will be checked how they fit the mcm "
                 "maps and result written to outfiles.");
   //
   // EXTRA DATA ON MERGED MAPS (  / --extradataonmergedmaps )
   //---------------------------------------------------------------------
   coh.addOption("", "--extradataonmergedmaps",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_applyExtraDataOnMergedMaps, "F",
                 "Apply extra data dynamic. The extra data specified "
                 "with -x /--extradata is applied on merged maps. Any "
                 "changes in underviewmaps are applied also to overview "
                 "and country overview maps.");


   //
   // Apply WASP/ED changes on merged maps, dynamic wasp/ed
   //---------------------------------------------------------------------
   coh.addOption("", "--wasponmergedmaps",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_applyWaspOnMergedMaps, "",
                 "Use this option to apply WASP/ED changes dynamically. "
                 "Give the outfile from the dynamic WASPing/ED on underview "
                 "maps as inparam "
                 "to apply the changes to overview and country overview "
                 "maps, all levels. The WASP/ED file (modifiedPOIs.txt or "
                 "modifiedItemsED.txt) has this structure:\n"
                 "mapId;itemId\n"
                 "mapId;itemId\n"
                 "...\n"
                 "The mapId = decimal id of the underview");
   
   //
   // Get latest extradata time per country
   //---------------------------------------------------------------------
   coh.addOption("", "--getEDtimePerCountry",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_getEDtimePerCountry, "",
                 "Loop the maps in this directory using startAtMap "
                 "and endAtMap (specified or default (0 resp MAX_UINT32)), "
                 "and find the latest extra data time per country. "
                 "If dynamic extra data time exists it is used, else the true "
                 "creation time of the map is used. The outfile is written "
                 "per country and can be used as inparam for extracting "
                 "dynamic extradata with ExtradataExtractor, e.g.\n"
                 " Sweden;2004-02-27 09:15:51\n"
                 " Norway;2004-01-31 00:31:51");

   //
   // Rename the mcm map
   //---------------------------------------------------------------------
   coh.addOption("-n", "--relabel",
                 CommandlineOptionHandler::uint32Val,
                 2, &CL_relabel, strmaxuint32.c_str(),
                 "Change the map ID of map u1 to u2. "
                 "Is not only changing the name of the map file, "
                 "but also all id information stored in the map file "
                 "header etc.");

   //
   // Start at map
   //---------------------------------------------------------------------
   coh.addOption("-E", "--startAtMap",
                 CommandlineOptionHandler::uint32Val,
                 1, &CL_startAtMap, strzero.c_str(),
                 "Start processing maps at the specified map id(decimal). "
                 "This parameter is used e.g. when -e or -x or -r "
                 "parameter is set.");

   //
   // End at map
   //---------------------------------------------------------------------
   coh.addOption("", "--endAtMap",
                 CommandlineOptionHandler::uint32Val,
                 1, &CL_endAtMap, strmaxuint32.c_str(),
                 "End processing maps at the specified map id (decimal) "
                 "= the last map to process is CL_endAtMap. "
                 "Used e.g. when -x or -r parameter is set.");

   //
   // Tools
   //---------------------------------------------------------------------
   coh.addOption("-@", "--tool",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_tool, "",
                 "Use GenerateMapServer as a tool. See "
                 "GMSTool::processMap for more info of options to give as "
                 "value. Use following command to get sub option "
                 "documentation:\n"
                 "grep \"^   [ ]*\\/\\*\\*\\|^   [ ]*\\*[ ]\\|^   "
                 "[ ]*\\*\\/\\|"
                 "option ==\" Server/MapGen/MapGenerator/src/GMSTool.cpp");
   
   //
   // Print items to mid/mif
   //---------------------------------------------------------------------
   coh.addOption("", "--printMidMif",
                 CommandlineOptionHandler::stringVal,
                 1,&CL_printMidMif,"",
                 "Print one kind of item from the mcm maps given in tail "
                 "to mid/mif file (file name without extension= option).\n"
                 "Note! The item must be in the file name, to know "
                 "which items to print.");
   //
   // Print map gfxdata to mid/mif
   //---------------------------------------------------------------------
   coh.addOption("", "--printMidMifMapGfx",
                 CommandlineOptionHandler::stringVal,
                 1,&CL_printMidMifMapGfx,"",
                 "Print a map gfxdata to a mid/mif file (file name "
                 "without extension= option). Either the gfxdata of "
                 "the map or the gfxdata of the municipal items in "
                 "the map can be printed. The latter choice is currently "
                 "hard coded. If a municipal has no gfxdata, it is "
                 "built as the convex hull of all street segments "
                 "in the municipal.\n"
                 "Specify maps in tail, or use startAtMap and endAtMap if "
                 "the maps are in this directory.");
   
   //
   // Print and test methods
   //---------------------------------------------------------------------
   coh.addOption("", "--loadM3Map",
                 CommandlineOptionHandler::uint32Val,
                 1, &CL_m3MapID, strmaxuint32.c_str(),
                 "Load an m3 map to see if it is OK." );

   coh.addOption("", "--listIndex",
                 CommandlineOptionHandler::presentVal,
                 0, &CL_listIndex, "",
                 "Loads index.db and lists the content of it to standard "
                 "out.");

   coh.addOption("", "--printHeader",
                 CommandlineOptionHandler::presentVal,
                 1, &CL_printHeader, "",
                 "Prints map header, "
                 "of the mcm maps in the tail.");


}


bool
commandLineSemantics(CommandlineOptionHandler& coh,
                     StringTable::countryCode& countryCode)
{
   bool ok = true;

   // check if tail present
   if(coh.getTailLength() > 0) {
      // 
      // Ok, we're going to process given rawdata file(s)
      //
      ;
   }
   else {
      if ( !CL_regenerateTurnDescriptions && 
           !CL_setLocations && 
           (CL_readExtraData == NULL) &&
           (CL_addrPointFile == NULL) &&
           !CL_finishIndexAreas &&
           !CL_extconnections &&
           !CL_addItemsFromExtraItemMaps &&
           !CL_createCountryMap &&
           !CL_addZipToCountryMap &&
           (CL_createItemsFromMidMif == NULL) &&
           (CL_initMapFromMidMif == NULL) &&
           !CL_generateStreetsFromStreetSegments &&
           (CL_printMidMif == NULL) &&
           (CL_printMidMifMapGfx == NULL) &&
           (CL_applyWaspOnMergedMaps == NULL) &&
           (CL_getEDtimePerCountry == NULL) &&
           !CL_createBorderBoundrySegments &&
           !CL_gfxFilteringForCoPols  &&
           !CL_createBorderItems &&
           !CL_createNewIndex &&
           !CL_listIndex &&
           !CL_addRegionsToIndex &&
           !CL_addMapSupCoverage &&
           !CL_updateCreationTimes &&
           !CL_createOverview &&
           !(CL_relabel[0] != MAX_UINT32) &&
           !(CL_m3MapID != MAX_UINT32) &&
           (CL_removeDupItemsFromMidMif == NULL) 
           )
      {
         cerr << "Nothing to do!" << endl;
         exit(0);
      }
   }

   
   // Set the country code   
   countryCode = MapGenUtil::getCountryCodeFromGmsName(CL_country);
   if ( countryCode == StringTable::NBR_COUNTRY_CODES ){
      mc2log << error << "Invalid GMS name, exits." << endl;
      MC2_ASSERT(false);
  }
   

   return ok;
}

bool fileOk(MC2String& fileName, readFile_t& fileType)
{
   MC2String _fileName = fileName;
   if(_fileName.size() < 5) {
      // 
      // Ok, this is a very short filename, probably without
      // suffix.
      //
      fileType = UNKNOWN;
   }
   else {
      //
      // Check the filename suffix for file type.
      //

      // .mcm.bz2
      MC2String suffixLong = _fileName.substr(_fileName.size() - 8, 8);
      // .mcm .gdf .xml etc
      MC2String suffix = _fileName.substr(_fileName.size() - 4, 4);
      if(suffixLong == ".mcm.bz2") {
         fileType = mcm;
      } 
      else if(suffix == ".gdf" || suffix == ".GDF")
         fileType = gdf;
      else if(suffix == ".mcm" || suffix == ".MCM")
         fileType = mcm;
      else if(suffix == ".xml" || suffix == ".XML")
         fileType = xml;

      if(fileType == UNKNOWN) {
         mc2log << error << "Know nothing about '" << fileName << "'" << endl;
         return false;
      }
      
      ifstream test(_fileName.c_str());
   }

   return true;
}

bool processXMLData( CommandlineOptionHandler* coh )
{
   // --------------------------------------------------------------------
   // First get all xml-files in the tail.

   set<const char*> xmlFiles;

   for (uint32 i = 0; i < coh->getTailLength(); i++ ) {
      const char* tail = coh->getTail( i );
      if ( StringUtility::endsWithStr( tail, ".xml" ) ) { 
         xmlFiles.insert( tail );
      }
   }

   // --------------------------------------------------------------------
   // Initialize the XML system
   try {
      XMLPlatformUtils::Initialize();
   } 
   catch(const XMLException& toCatch) {
      cerr << "Error during Xerces-c Initialization.\n"
           << "  Exception message:"
           << toCatch.getMessage() << endl;
      return false;
   }
 
   XMLParserHelper::domStringSet_t toSortByOrder;
   XMLParserHelper::domStringSet_t toSortByTag;

   XStr tmpDOMStr( "map_generation-mc2" );
   XStr createOverview( "create_overviewmap" );
   toSortByTag.insert( createOverview.XMLStr() );
   
   XMLParserHelper helpParser( tmpDOMStr.XMLStr(),
                               toSortByOrder,
                               toSortByTag );

   if ( ! helpParser.load( xmlFiles ) ) {
      mc2log << warn 
             << " GenerateMapServer::processXMLData Failed loading "
             << "xmlFiles." << endl;
      return false;
   }

   XMLParserHelper::xmlByTag_t xmlByTag = helpParser.getXMLNodesByTag();
  
   
   // --------------------------------------------------------------------
   // Create overview maps.
   
   pair<XMLParserHelper::xmlByTag_t::iterator, 
        XMLParserHelper::xmlByTag_t::iterator> createOverviewRange =
           xmlByTag.equal_range( createOverview.XMLStr() );
               
   if ( createOverviewRange.first != createOverviewRange.second ) {
      // Create overview maps.
      
      // The underview maps that make up overview maps. 
      map<MC2String, MC2String> overviewByUnderview;
      
      // Parse xml
      for ( XMLParserHelper::xmlByTag_t::iterator it = 
               createOverviewRange.first;
            it != createOverviewRange.second; ++it ) {
         XMLIndata::parseCreateOverview( 
               it->second, overviewByUnderview );
      }
      
      // Create overview maps.
      createCountryMaps( coh, overviewByUnderview );


   }

   // Shut down the XML system, will cause a seg-fault due to
   // multiple deletes of the same object, so do not terminate!
   //XMLPlatformUtils::Terminate();
   return true;
}



void updateExternalConnections() 
{
   mc2dbg << "updateExternalConnections startAtMap=" << CL_startAtMap 
          << " endAtMap=" << CL_endAtMap << endl;
   if ( CL_processNeighbours ){
      mc2dbg << "Will process neighbouring maps." << endl;
   }
   
   uint32 curMapID = 0; // Needs to check all maps. //CL_startAtMap;
   if (MapBits::isOverviewMap(CL_startAtMap)){
      if (MapBits::getMapLevel(CL_startAtMap) > 1){
         curMapID=FIRST_SUPEROVERVIEWMAP_ID;
      }
      else {
         curMapID=FIRST_OVERVIEWMAP_ID;
      }
   }
   else if (MapBits::isCountryMap(CL_startAtMap)){
      curMapID=FIRST_COUNTRYMAP_ID;
   }
   mc2dbg1 << "updateExternalConnections curMapID=" << curMapID << endl;

   uint32 nbrMaps = 0;
   bool cont = true;

   // The danglingEndsByMap_t is typedef of
   //       map<uint32, map<uint32, GMSMap::extdata_t>* >
   // mapID - nodeID - GMSMap::extdata_t
   danglingEndsByMap_t danglingEndsByMap;
   
   const char* mapPath = "./";

   // Read all the maps and get the boundry segments
   mc2log << info << "Read all the maps and get the boundry segments" << endl;
   while (cont) {
      OldGenericMap* curMap = OldGenericMap::createMap(curMapID, mapPath);
      if (curMap != NULL) {
         // Print status
         mc2dbg1 << "Reading boundry segments for map with ID " 
                 << curMapID << endl;

         OldBoundrySegmentsVector* boundrySegments = 
            curMap->getBoundrySegments();
         if (boundrySegments == NULL) {
            mc2log << warn << here << " No boundry segments" << endl;
         } else {
            mc2dbg1 << " nbrItemsToAdd = " << boundrySegments->getSize() 
                    << endl;
            for (uint32 i=0; i<boundrySegments->getSize(); i++) {
               uint32 curItemID = ((OldBoundrySegment*) boundrySegments->
                     getElementAt(i))->getConnectRouteableItemID();
               OldRouteableItem* ri = static_cast<OldRouteableItem*>
                  (curMap->itemLookup(curItemID));
               ItemTypes::itemType type = ri->getItemType();
               mc2dbg2 << "    itemID = " << curItemID 
                       << " type " << StringTable::getString(
                         ItemTypes::getItemTypeSC(type), StringTable::ENGLISH)
                       << endl;
               OldBoundrySegment::closeNode_t closeNodeVal =
                  ((OldBoundrySegment*)boundrySegments->getElementAt(i))
                   ->getCloseNodeValue();

               //The extdata where we will store the result
               GMSMap::extdata_t data;
               data.mapID = curMapID;
               data.roadClass = ri->getRoadClass();
               data.type = type;
               if (type == ItemTypes::streetSegmentItem) {
                  OldStreetSegmentItem* ssi = 
                     static_cast<OldStreetSegmentItem*> (ri);
                  data.ramp = ssi->isRamp();
                  data.roundabout = ssi->isRoundabout();
                  data.multiDig = ssi->isMultiDigitised();
               } else if (type == ItemTypes::ferryItem) {
                  data.ramp = false;
                  data.roundabout = false;
                  data.multiDig = false;
               } else {
                  mc2log << fatal << here << "Boundry segment is"
                         << " not a routeable item, is type "
                         << int (type) << endl;
                  exit (1);
               }

               GfxData* curGfx = ri->getGfxData();
               // curNode is the node close to the boundry
               OldNode* curNode = NULL;
               OldNode* otherNode = NULL;
               uint32 n = 0;
               switch (closeNodeVal) {
                  case OldBoundrySegment::node0close :
                     mc2dbg4 << "Adding OldNode 0" << endl;
                     curNode = ri->getNode(0);
                     otherNode = ri->getNode(1);
                     break;
                  case OldBoundrySegment::node1close :
                     mc2dbg4 << "Adding OldNode 1" << endl;
                     curNode = ri->getNode(1);
                     otherNode = ri->getNode(0);
                     n = curGfx->getNbrCoordinates(0)-1;
                     break;
                  default:
                     mc2log << fatal << here << " Unknown closeNodeVal " 
                            << int(closeNodeVal) << ", exiting!" << endl;
                     exit(1);
               }

               data.nodeID = curNode->getNodeID();
               data.lat = curGfx->getLat(0,n);
               data.lon = curGfx->getLon(0,n);
               data.entryRestrictions = curNode->getEntryRestrictions();
               data.level = curNode->getLevel();
               
               // To calculate the angle of the bs, use coordinates of 
               // the neighbouring routeable item.
               if ( otherNode->getNbrConnections() != 1 ){
                  mc2log << error << "Other node's number of connections "
                         << "!= 1. Nbr conns: "
                         << otherNode->getNbrConnections()
                         << " NodeID: " << otherNode->getNodeID()
                         << endl;
                  exit(1);
               }
               uint32 prevNodeID = 
                  (otherNode->getEntryConnection(0)->
                   getConnectFromNode()) ^ 0x80000000;
               OldNode* prevNode = curMap->nodeLookup(prevNodeID);
               GfxData* prevGfx = curMap->itemLookup(prevNodeID)->getGfxData();
               uint32 prevCoordIndex = 1;
               if (!prevNode->isNode0()) {
                  prevCoordIndex = prevGfx->getNbrCoordinates(0) - 2;
               }
               MC2_ASSERT(prevCoordIndex < prevGfx->getNbrCoordinates(0));
               data.angle = GfxUtility::getAngleFromNorth(
                              prevGfx->getLat(0,prevCoordIndex),
                              prevGfx->getLon(0,prevCoordIndex),
                              curGfx->getLat(0,n),
                              curGfx->getLon(0,n));
               data.endNodeAngle = 0;
              
               // Add the data to the danglingEnds
               if ( danglingEndsByMap.find(data.mapID) == 
                    danglingEndsByMap.end() )
               {
                  map<uint32, GMSMap::extdata_t>* nodeDataByNodeId =
                     new map<uint32, GMSMap::extdata_t>();
                  danglingEndsByMap.insert(pair<uint32, 
                                           map<uint32, GMSMap::extdata_t>*>
                                           (data.mapID,
                                            nodeDataByNodeId) );
               }
               map<uint32, GMSMap::extdata_t>* currMapNodeDataByNodeId =
                  danglingEndsByMap.find(data.mapID)->second;
               currMapNodeDataByNodeId->
                  insert(pair<uint32, GMSMap::extdata_t >
                         (data.nodeID, data));
                                               
                                               
            }
            mc2dbg1 << "All items added." << endl;
         }

         // Increase the mapID-variable
         nbrMaps++;
         curMapID = MapBits::nextMapID(curMapID);
      } else {
         cont = false;
      }

      // Delete the map to avoid memory leaks
      delete curMap;
   }

   // Create bounding boxes for each map from the boundry segments.
   mc2log << info 
          << "Create bounding boxes for each map from the boundry segments"
          << endl;
   danglingEndsByMap_t::const_iterator mapDlgIt =
      danglingEndsByMap.begin();
   map<uint32, MC2BoundingBox*> bBoxByMap;
   while (mapDlgIt != danglingEndsByMap.end()){
      uint32 mapID = mapDlgIt->first;
      MC2BoundingBox* bBox = new MC2BoundingBox();
      bBoxByMap.insert(pair<uint32, MC2BoundingBox*>(mapID, bBox));

      map<uint32, GMSMap::extdata_t>* danglingEnds = mapDlgIt->second;
      map<uint32, GMSMap::extdata_t>::const_iterator dlgIt = 
         danglingEnds->begin();
      while (dlgIt != danglingEnds->end()){
         GMSMap::extdata_t data = dlgIt->second;
         bBox->update(data.lat, data.lon);
         dlgIt++;
      }

      // Increse the bounding box so touching bounding boxes intersect.
      bBox->increaseMeters( 100 );

      mc2dbg << "BBox for map: " << mapID << endl;
      bBox->dump();
      mapDlgIt++;
   }

   // Create multimap for overlapping bounding boxes;
   mc2log << info << "Check for maps close enough to look for external"
          << " connections to" << endl;
   multimap<uint32, uint32> surroundingMapsByMap;
   map<uint32, MC2BoundingBox*>::const_iterator mapIt = bBoxByMap.begin();
   while (mapIt != bBoxByMap.end()){
      uint32 currMapId = mapIt->first;
      MC2BoundingBox* currBBox = mapIt->second;
      
      map<uint32, MC2BoundingBox*>::const_iterator bBoxIt = 
         bBoxByMap.begin();
      while (bBoxIt !=  bBoxByMap.end()){
         uint32 tmpMapId = bBoxIt->first;
         MC2BoundingBox* tmpBBox = bBoxIt->second;   
         
         if (currBBox->overlaps( *tmpBBox ) && currMapId != tmpMapId ){
            surroundingMapsByMap.insert(pair<uint32, uint32>
                                        ( currMapId, tmpMapId ) );
            mc2log << info << "Map: " << currMapId 
                   << "'s bbox is close to map: " << tmpMapId << endl;
         }
         bBoxIt++;
      }
      mapIt++;
   }

   map<uint32, MC2BoundingBox*>::iterator bbIt = bBoxByMap.begin();
   while (bbIt != bBoxByMap.end()){
      delete bbIt->second;
      bbIt->second = NULL;
      bbIt++;
   }

   // Loop over all the maps once again and update the external connections
   mc2log << info << "Loop all maps and now update the external connections" << endl;
   cont = true;
   curMapID = CL_startAtMap;
   set<uint32>firstProcessedMapIDs;
   while (cont) {
      OldGenericMap* curMap = OldGenericMap::createMap(curMapID, mapPath);
      if ( (curMap != NULL ) &&
           (curMap->getMapID() <= CL_endAtMap) ) {
         // Print status
         mc2dbg1 << "Adding connections to map with ID " << curMapID 
                 << endl;
         
         updExtConnsOfMap(curMap, danglingEndsByMap, surroundingMapsByMap);
       
         // Save the map and mark it as processed
         curMap->save();
         firstProcessedMapIDs.insert(curMap->getMapID());

         // Increase the mapID-variable
         curMapID = MapBits::nextMapID(curMapID);
      } else {
         cont = false;
      }

      // Delete the map to avoid memory leaks
      delete curMap;
   }


   if (CL_processNeighbours){

      // Will keep track so we don't process same map multiple times.
      // First copy the ones already processed, later fill it with the
      // processed neighbours.
      set<uint32>processedMapIDs;
      set<uint32>::const_iterator mapIdIt = firstProcessedMapIDs.begin();
      while ( mapIdIt != firstProcessedMapIDs.end()){      
         processedMapIDs.insert(*mapIdIt);
         ++mapIdIt;
      }


      mc2log << info << "Handling neighbouring maps." << endl;
      mapIdIt = firstProcessedMapIDs.begin();
      while ( mapIdIt != firstProcessedMapIDs.end()){
         
         multimap<uint32, uint32>::const_iterator surrMapsIt = 
            surroundingMapsByMap.lower_bound(*mapIdIt);
         while (surrMapsIt != 
                surroundingMapsByMap.upper_bound(*mapIdIt))    {
            set<uint32>::const_iterator searchIt = 
               processedMapIDs.find(surrMapsIt->second);
            if ( searchIt == processedMapIDs.end() ){
               // This map is a neighbouring/surronding map and it has not
               // been processed before. Create external connections for 
               // it.

               OldGenericMap* curMap = 
                  OldGenericMap::createMap(surrMapsIt->second, mapPath);
               if (curMap == NULL){
                  mc2log << error << "Could not open " 
                         << surrMapsIt->second
                         << " exits!" << endl;
                  exit(1);
               }
               mc2log << info 
                      << "Adding connections to neighbour map with ID 0x" 
                      << hex << surrMapsIt->second << dec << "("
                      << surrMapsIt->second << ")" << endl;
               updExtConnsOfMap(curMap, danglingEndsByMap, 
                                surroundingMapsByMap);
               
               // Save the map
               curMap->save();
               processedMapIDs.insert(curMap->getMapID());
               delete curMap;
            }
            else {
               mc2dbg << "Map 0x" << hex << surrMapsIt->second << dec 
                      << " already processed." << endl;
            }
            
            ++surrMapsIt;
         }
         ++mapIdIt;
      }
      mc2log << info << "Handling neighbouring maps done!" << endl;
   }
   


   // Clear dangling ends by map.
   danglingEndsByMap_t::iterator it
      = danglingEndsByMap.begin();
   while (it != danglingEndsByMap.end()){
      delete it->second;
      it->second = NULL;
      it++;
   }
} // updateExternalConnections (all maps in current directory)

void updExtConnsOfMap(OldGenericMap* curMap, 
                      danglingEndsByMap_t& danglingEndsByMap,
                      multimap<uint32, uint32>& surroundingMapsByMap)
{

   uint32 curMapID = curMap->getMapID(); 

   //vector to count the number of added con from other maps
   typedef map<uint32, uint32> u32map;
   u32map nbrAddedConnFromMap;

   // Get the boundrySegments Vector
   OldBoundrySegmentsVector* bsVec = curMap->getBoundrySegments();
   
   if (bsVec == NULL) {
      mc2log << warn << here << " No boundry segments" << endl;
   } else {

      // Dump the current bs-vec
      bsVec->dump();
      // Remove all the external connections of this map!
      bsVec->deleteAllConnections();



      // Check data of the boundry segments of the maps surrounding 
      // this map against it.
      multimap<uint32, uint32>::const_iterator surrMapsIt = 
         surroundingMapsByMap.lower_bound(curMapID);
      while (surrMapsIt != 
             surroundingMapsByMap.upper_bound(curMapID))
      {
         uint32 surrMapId=surrMapsIt->second;
         mc2log << info << "Checking against map with ID:" 
                << surrMapId << endl;
         danglingEndsByMap_t::const_iterator tmpIt = danglingEndsByMap.find(surrMapId);
         if (tmpIt != danglingEndsByMap.end()){
            map<uint32, GMSMap::extdata_t>* surrMapDlgEnds = 
               tmpIt->second;
            mc2dbg << "Map " << surrMapId << " has "
                   << surrMapDlgEnds->size() << " virtuals" << endl;
            map<uint32, GMSMap::extdata_t>::const_iterator it = 
               surrMapDlgEnds->begin();
            while (it != surrMapDlgEnds->end()){
               if (it->second.mapID != curMapID) {
                  // Try to add a external connection
                  if (nbrAddedConnFromMap.find(it->second.mapID) ==
                      nbrAddedConnFromMap.end())
                  {
                     nbrAddedConnFromMap[it->second.mapID] =
                        GMSUtility::addExternalConnection(curMap,
                                                          it->second );
                  } else {
                     nbrAddedConnFromMap[it->second.mapID] +=
                        GMSUtility::addExternalConnection(curMap,
                                                          it->second );
                  }
               }
               it++;
            }
         }
         surrMapsIt++;
      }


      // Print the number of added con
      mc2log << info << "Added external connections to map " 
             << curMapID << endl;
      
      uint32 key = 0;
      u32map::iterator theIt = 
         nbrAddedConnFromMap.lower_bound(key);
      while (theIt != nbrAddedConnFromMap.end()) {
         key = theIt->first;
         if (key != curMapID) {
            mc2log << info << "    from map " << key
                       << ": " << nbrAddedConnFromMap[key] << endl;
         }
         theIt = nbrAddedConnFromMap.upper_bound(key);
      }


      // Set turndescriptions for the external connections
      mc2dbg1 << "To set turndescriptions for external connections "
              << "to this map with id=" << curMapID << endl;
      Vector dummyVec;
      ObjVector checkedBoundrySegments;
      const uint32 maxNbrNodeIDs = 25;
      for (uint32 j = 0; j < bsVec->getSize(); j++) {
         OldBoundrySegment* bs = 
            dynamic_cast<OldBoundrySegment*>(bsVec->getElementAt(j));
         if (checkedBoundrySegments.binarySearch(bs) == MAX_UINT32) {
            ItemTypes::itemType myType =
               curMap->itemLookup(bs->getConnectRouteableItemID())
               ->getItemType();
            mc2dbg4 << "Checking bs " << curMapID << ":"
                    << bs->getConnectRouteableItemID() 
                    << " type=" << int(myType) 
                    << " nbrconns[0]=" << bs->getNbrConnectionsToNode(0)
                    << " nbrconns[1]=" << bs->getNbrConnectionsToNode(1)
                    << endl;
            ObjVector neighbouringBS;
            neighbouringBS.addLast(bs);
            ObjVector* crossingElements = new ObjVector();
            byte nodeNbr = 0;
            // Assume not more than maxNbrNodeIDs nodes in a crossing
            uint32 nodeIDs[ maxNbrNodeIDs ]; 
            // Find other bs closeNodes (in other maps)
            // that are connected to the opposite node
            // of this bs' closeNode (the not_close node)
            // and build crossingElementNotice.
            uint32 n;
            if (bs->getCloseNodeValue() == OldBoundrySegment::node0close){
               n = 0;
            } else if (bs->getCloseNodeValue() == 
                       OldBoundrySegment::node1close) {
               n = 1;
            } else {
               mc2log << error << "Neither node 0 nor node 1 is closest"
                      << "closeNodeVal=" << int(bs->getCloseNodeValue()) 
                      << endl;
               n = 0;
            }
            mc2dbg8 << " node_close " << n << endl;
            for (uint32 i = 0; i < bs->getNbrConnectionsToNode(n); i++) {
               OldConnection* con = bs->getConnectToNode(n, i);
               uint32 connectFromMapID = bs->getFromMapIDToNode(n, i);
               uint32 connectFromNodeID = con->getConnectFromNode();
               connectFromNodeID ^= 0x80000000;

               typedef map<uint32, GMSMap::extdata_t>::iterator DEIT;
               DEIT it;
               danglingEndsByMap_t::const_iterator tmpIt = 
                  danglingEndsByMap.find(connectFromMapID);
               if (tmpIt != danglingEndsByMap.end()){
                  map<uint32, GMSMap::extdata_t>* dlgEnds =
                     tmpIt->second;
                  it = dlgEnds->find(connectFromNodeID);
                  if (it == dlgEnds->end()){
                     MC2_ASSERT(false);
                  }
               }
               else {
                  MC2_ASSERT(false);
               }

               // iterator it holds data for the dangling end
               mc2dbg8 << "Found dangling end," << it->second.mapID 
                       << "." << it->second.nodeID << endl;

               // So really, here we could set enter/exit ferry
               // for the ext conn if the it->second.type
               // differs from my item type
               // How to handle change-ferry?
               if ( (myType == ItemTypes::streetSegmentItem) &&
                    (it->second.type == ItemTypes::ferryItem) ) {
                  con->setTurnDirection( ItemTypes::EXIT_FERRY );
               } else if ( 
                  (myType == ItemTypes::ferryItem) &&
                  (it->second.type == ItemTypes::streetSegmentItem) ) {
                  con->setTurnDirection( ItemTypes::ENTER_FERRY );
               }
               if ( con->getTurnDirection() != ItemTypes::UNDEFINED ) {
                  mc2dbg8 << "  td from " << connectFromMapID << ":"
                          << connectFromNodeID << " = "
                          << StringTable::getString(
                           ItemTypes::getTurndirectionSC(
                              con->getTurnDirection()),
                           StringTable::ENGLISH) << endl;
               }

               // Add to crossingElementNoticeArray
               // Known memory leak!
               uint32 added = crossingElements->addLastIfUnique(
                     new CrossingConnectionElement(
                        it->second.angle,
                        it->second.endNodeAngle,
                        nodeNbr,
                        it->second.roadClass,
                        it->second.roundabout,
                        it->second.ramp,
                        it->second.multiDig,
                        dummyVec,
                        it->second.entryRestrictions));
               if (added != MAX_UINT32) {
                  mc2dbg8 << "  Crossing elem " << connectFromMapID
                     << "." << connectFromNodeID << ": "
                     << it->second.angle << " "
                     << it->second.endNodeAngle << " "
                     << int(nodeNbr) << " "
                     << int(it->second.roadClass) << " " 
                     << it->second.roundabout << " " 
                     << it->second.ramp << " "
                     << it->second.multiDig << " "
                     << int(it->second.entryRestrictions)
                     << endl;
                  
                  if ( nodeNbr >= maxNbrNodeIDs ) {
                     mc2log << error << nodeNbr
                        << " >= maxNbrNodeIDs"
                        << ", nodeNbr = " << nodeNbr << endl;
                     MC2_ASSERT( false );
                  } 
                  nodeIDs[nodeNbr] = connectFromNodeID;
                  
                  nodeNbr++;
               }
               // Collect the boundry segments in this map that have 
               // connection from the node on the other map
               // (i.e get the "neighbours" of this bs)
               bsVec->getBoundrySegments(connectFromMapID,
                     connectFromNodeID, &neighbouringBS);
            }

            // Build crossingElementNotice of the close_node of the 
            // elements in neighbouringBS (no need to check 
            // IfUnique, already checked in neighbouringBS)
            for (uint32 k = 0; k < neighbouringBS.getSize(); k++) {
               OldBoundrySegment* curBS = dynamic_cast<OldBoundrySegment*>
                  (neighbouringBS.getElementAt(k));
               uint32 closeN = 0;
               if (curBS->getCloseNodeValue() == 
                     OldBoundrySegment::node0close)
                  closeN = 0;
               else if (curBS->getCloseNodeValue() == 
                     OldBoundrySegment::node1close)
                  closeN = 1;
               uint32 itemID = curBS->getConnectRouteableItemID();
               OldRouteableItem* ri = static_cast<OldRouteableItem*>
                  (curMap->itemLookup(itemID));
               uint32 curBSNodeID = ri->getNode(closeN)->getNodeID();
               typedef map<uint32, GMSMap::extdata_t>::iterator DEIT;
               DEIT it;
               danglingEndsByMap_t::const_iterator tmpIt = 
                  danglingEndsByMap.find(curMap->getMapID());
               if (tmpIt != danglingEndsByMap.end()){
                  map<uint32, GMSMap::extdata_t>* dlgEnds =
                     tmpIt->second;
                  it = dlgEnds->find(curBSNodeID);
                  if (it == dlgEnds->end()){
                     MC2_ASSERT(false);
                  }
               }
               else {
                  MC2_ASSERT(false);
               }

               mc2dbg8 << "Found bsNode " << it->second.nodeID 
                       << " of bs " << itemID << endl;
               crossingElements->addLast(new CrossingConnectionElement(
                        it->second.angle,
                        it->second.endNodeAngle,
                        nodeNbr,
                        it->second.roadClass,
                        it->second.roundabout,
                        it->second.ramp,
                        it->second.multiDig,
                        dummyVec,
                        it->second.entryRestrictions));
               mc2dbg8 << "  Crossing elem " << itemID << ": "
                  << it->second.angle << " "
                  << it->second.endNodeAngle << " "
                  << int(nodeNbr) << " "
                  << int(it->second.roadClass) << " " 
                  << it->second.roundabout << " " 
                  << it->second.ramp << " "
                  << it->second.multiDig << " "
                  << int(it->second.entryRestrictions)
                  << endl;
               if ( nodeNbr >= maxNbrNodeIDs ) {
                  mc2log << error << nodeNbr
                     << " >= maxNbrNodeIDs"
                     << ", nodeNbr = " << nodeNbr << endl;
                  MC2_ASSERT( false );
               }
               nodeIDs[nodeNbr] = curBSNodeID;
               
               nodeNbr++;
            }

            if (nodeNbr > 1) {
               // If at least two crossing connection elements,
               // we should calculate turndescriptions
               
               // First sort the vector with crossing connection elements
               // and build a sortedNodeIDs vector
               uint32 sortedNodeIDs[ maxNbrNodeIDs ];
               crossingElements->sort();
               for (uint32 i = 0; i < nodeNbr; i++) {
                  CrossingConnectionElement* thisElem = 
                     static_cast<CrossingConnectionElement*>
                     (crossingElements->getElementAt(i));
                  if ( i >= maxNbrNodeIDs ) {
                     mc2log << error << i 
                        << " >= maxNbrNodeIDs"
                        << ", nodeNbr = " << nodeNbr << endl;
                     MC2_ASSERT( false );
                  }
                  sortedNodeIDs[i] = nodeIDs[thisElem->m_nodeIndex];
               }

               // Then calculate turn descriptions
               typedef vector<Crossing::turndesc_t> turndescvector_t;
               turndescvector_t result;
               ItemTypes::crossingkind_t kind;
               bool rightSideTraffic = curMap->driveOnRightSide();
               bool calcTurns = Crossing::getTurnDescriptions(
                     crossingElements,
                     rightSideTraffic,
                     result,
                     kind,
                     StringTable::NBR_COUNTRY_CODES); // no specific
                                                      // country
               mc2dbg8 << "Calculated turndescriptions " 
                       << calcTurns << endl;

               typedef vector<Crossing::turndesc_t>::iterator RIT;
               DEBUG4(
               for (RIT it = result.begin(); it != result.end(); it++) {
                  mc2dbg4 << "From " << int(it->fromConIndex)
                          << " to " << int(it->toConIndex)
                          << " turn " << int(it->description)
                          << endl;
               });
               // This didn't calculate
               // - enterFerry, exitFerry, changeFerry
               // It gives
               // - straightAhead, turnLeft+Right, keepLeft+Right

               // Set turndescriptions for connections to segments in 
               // neighbouringBS ( = bs and its "neighbours")
               //typedef vector<Crossing::turndesc_t>::iterator RIT;
               for (uint32 k = 0; k < neighbouringBS.getSize(); k++) {
                  OldBoundrySegment* curBS = dynamic_cast<OldBoundrySegment*>
                     (neighbouringBS.getElementAt(k));
                  uint32 curBScloseNbr = 0;
                  uint32 curBScloseID = 
                         curBS->getConnectRouteableItemID();
                  if (curBS->getCloseNodeValue() == 
                        OldBoundrySegment::node1close) {
                     curBScloseNbr = 1;
                     curBScloseID ^= 0x80000000;
                  }
                  for (RIT it = result.begin(); 
                                 it != result.end(); it++) {
                     if (sortedNodeIDs[int(it->toConIndex)] 
                         == curBScloseID){
                        //We have a struct with info to a node of curBS
                        // let the connections to both nodes of curBS get 
                        // info from the struct
                        for (uint32 n = 0; n < 2; n++) {
                           for (uint32 c = 0;
                                c < curBS->getNbrConnectionsToNode(n);
                                c++) {
                              OldConnection* curCon = 
                                 curBS->getConnectToNode(n, c);
                              uint32 fromNodeID = 
                                 curCon->getConnectFromNode();
                              if (n == curBScloseNbr) {
                                 fromNodeID ^= 0x80000000;
                              }
                              if (sortedNodeIDs[int(it->fromConIndex)]
                                  == fromNodeID){
                                 //the struct has info for this conn
                                 // don't override enter/exit ferry
                                 if ( curCon->getTurnDirection() ==
                                       ItemTypes::UNDEFINED) {
                                    curCon->
                                       setTurnDirection(it->description);
                                 }
                                 curCon->setExitCount(it->exitCount);
                                 curCon->setCrossingKind(kind);
                                 mc2dbg1  << " Setting turndir "
                                      << int(it->description) << "->"
                                      << int(curCon->getTurnDirection())
                                      << " for con " << c 
                                      << " (from " << fromNodeID << ")"
                                      << " to node " << n << " of bs "
                                      << curBS->
                                          getConnectRouteableItemID()
                                      << endl;
                              }
                           }//for c
                        }//for n
                     }//if
                  }//for it
               }
            }
            
            // Mark the checked nodes (of bs + neighbours)
            for (uint32 k = 0; k < neighbouringBS.getSize(); k++) {
               checkedBoundrySegments.addLast(
                     neighbouringBS.getElementAt(k));
            }
            checkedBoundrySegments.sort();

            delete crossingElements;
         }
      } // End Set turndescriptions for the external connections
   }



} // updExtConnsOfMap (for one map)

   


void
createCountryMapBorder( CommandlineOptionHandler* coh,
                        GMSCountryOverviewMap* curCountryMap){

   mc2log << info << "To create country map border" << endl;

   // Create the boundry for current country map. (reads from country.mif)
   if (!curCountryMap->createCountryBorder()) {
      mc2log << warn << here << " Could not create country boundry"
             << "   mapID=" << curCountryMap->getMapID() 
             << endl;
   } else {
      mc2dbg1 << "Boundry for country created, mapID=" 
              << curCountryMap->getMapID() << endl;
   }
   
   
   
   // Go through the maps in the tail and check if any 
   // country overview map with the same country code as curCountryMap exists.
   // In that case:
   // if the found co map has filtered map gfx data 
   //  - copy the gfx data to curCountryMap (coord filtering copied)
   //  - set setMapGfxDataFiltered in curCountryMap
   //  - create the countryBorder.txt file using copolbreakpoints
   //  - create simplified gfx representation
   //
   // if the found co map does not have filtered map gfx
   //  - the found map is no good..
   //  - filter copol coordinates in curCountryMap
   //  - create simplified gfx representation
   
   mc2log << info << "Loop the tail to find matching old co map" << endl;
   if (coh != NULL) {
      // Linear search until we find a map with the same country code.
      bool found = false;
      uint32 i = 0;
      while ((i < coh->getTailLength()) && (! found)) {
         const char* tail = coh->getTail(i);
         // Make sure it's a .mcm file
         if ( StringUtility::endsWithStr( tail, ".mcm" ) ) {
            GMSCountryOverviewMap* filterMap = 
               dynamic_cast<GMSCountryOverviewMap*> 
               (GMSMap::createMap( tail ));
            if ( filterMap != NULL ){
               if ( strcmp( filterMap->getMapName(), 
                            curCountryMap->getMapName() ) == 0 )
               { 
                  if ( filterMap->mapGfxDataIsFiltered() ) {
                     if ( curCountryMap->copyCoPolFilteringFromMap(
                              filterMap, CL_countryPolygonBreakPoints) ) {
                        mc2dbg1 << "Copied country polygon filtering from "
                                << coh->getTail(i) << " to map "
                                << curCountryMap->getMapID() << endl;
                     } else {
                        mc2log << error << " while copying country polygon "
                               << "filtering from " << coh->getTail(i) 
                               << " to map " << curCountryMap->getMapID()
                               << endl;
                     }
                  } else {
                     // The found map is not filtered, filter this co map
                     mc2dbg1 << "The found map has no coordinate filtering "
                             << "in co pol - will filter here!" << endl;
                     // coordinate filtering
                     filterCoPolCoordinateLevels(
                           curCountryMap, CL_countryPolygonBreakPoints );
                     // gfx filtering
                     curCountryMap->makeSimplifiedCountryGfx();
                  }
                  // Exit loop.
                  found = true;
               }
            }
            else {
               mc2log << error << "GenerateMapServer::" 
                      << "createCountryMapBorder. Error opeing country"
                      << "map to fetch filtering from. Exits!" << endl;
               exit(1);
            }
            delete filterMap;
         }
         i++;
      }

      if ( !found ){
         // Could not find country map to copy filtering from.
         mc2dbg2 << "No matching co map in tail, filter now." << endl;
         // coordinate filtering
         filterCoPolCoordinateLevels(
               curCountryMap, CL_countryPolygonBreakPoints );
         // gfx filtering
         curCountryMap->makeSimplifiedCountryGfx();
      }
   }
   
   // We have used all info we can from co maps in tail for building 
   // the map gfx data of this curCountryMap regarding both 
   // coordinate filtering and gfx filtering
   
} // createCountryMapBorder



void 
createCountryMaps( CommandlineOptionHandler* coh, 
                   map<MC2String, MC2String>& overviewByUnderview )
{
   mc2log << info << " Start to create CountryMaps" << endl;
   bool oneMapAtATime = true;

 

   // Initiate some variables
   const char* countryMapPath = "./";
   const char* mapPath = "./";

   // OldMaps underview ID and handled status to underview map name.
   // Handled status is true if the map has been added to an overview map
   // and false otherwise.
   MC2String mapIDsByNameFile = "undName2MapID.txt";
   OldMapIdByName mapIDsByName( mapIDsByNameFile );
   if ( mapIDsByName.fileExists() ){
      mc2log << info << "GMS::createCountryMaps" 
             << " OldMapping map ID to name file found, using it." << endl;

      // Read underview names from file.
      bool result = mapIDsByName.readFile();
      if ( !result ){
         mc2log << error << "GMS::createCountryMaps: " 
                << "Failed to read underview to map ID file."
                << " Exits!" << endl;
         exit(1);
      }
   }
   else {
      // Create unverview map name to ID and handled status list and
      // write it to disk.
      mc2log << info << "GMS::createCountryMaps" 
             << " OldMapping map ID to name by reading from disk" << endl;
      bool result = mapIDsByName.initAndWriteFile( countryMapPath );
      if ( !result ){
         mc2log << error << "GMS::createCountryMaps: " 
                << "Failed to create underview to map ID file."
                << " Exits!" << endl;
         exit(1);
      }
   }

   // Check that all underview maps are present in the file.
   uint32 highestID = 0;
   uint32 nextMapID = 0;
   bool result = mapIDsByName.cmpUndIDsWithMapFiles( countryMapPath,
                                                     highestID,
                                                     nextMapID );
   if ( ! result ){
      // Indicates missing underview maps or present overview maps in 
      // countryMapPath.
      mc2log << error << "GMS::createCountryMaps: "
             << " OldMapIdByName::cmpUndIDsWithMapFiles failed" << endl;
      mc2dbg << "GMS::createCountryMaps: " << hex 
             << "(highestID+1) != nextMapID"
             << "highestID:" << highestID 
             << ", nextMapID:" << nextMapID << dec << " Exits!" << endl;
      exit(1);
   }
   OldMapIdByName::mapIdAndHdlByName_t& undIdAndHdlByName = 
      mapIDsByName.getWriteableMapIdByAndHdlName();


   // OldMap unverview ID to map name.
   map<uint32, MC2String> underviewNameByID; 
   OldMapIdByName::mapIdAndHdlByName_t::const_iterator nameIt = 
      undIdAndHdlByName.begin();
   while ( nameIt != undIdAndHdlByName.end() ){
      pair<uint32, MC2String> idAndName;
      idAndName.first = nameIt->second.first;
      idAndName.second = nameIt->first;

      // Second returned from insert is true if the key to be inserted
      // did not already exits.
      bool result = (underviewNameByID.insert(idAndName)).second;
      if ( ! result ){
         mc2log << error << "GMS::createCountryMaps: " 
                << "OldMap ID:" << idAndName.first 
                << " already inserted into underviewNameByID."
                << " Not good, exits!" << endl;
         exit(1);
      }
      ++nameIt;
   }
   
   // Determine process order of country overview maps.
   vector<MC2String>overviewOrder;  // Contains the country overview names.
   map<uint32, MC2String>::const_iterator undIt = underviewNameByID.begin();
   while ( undIt != underviewNameByID.end() ){

      MC2String underviewName = undIt->second;

      map< MC2String, MC2String>::const_iterator tmpIt = 
            overviewByUnderview.find(underviewName);
      if (tmpIt == overviewByUnderview.end()){
         mc2dbg << "Underview map: \"" << underviewName
                << "\" not included in ceated co ovrs" << endl;
      }
      else {
         MC2String overviewName=tmpIt->second;
         
         if ( find(overviewOrder.begin(), overviewOrder.end(),
                   overviewName ) == overviewOrder.end() )
            {
               // Not already added overview, add this one.
               overviewOrder.push_back(overviewName);
            }
      }
      ++undIt;
   }



   // Make the overview map the key instead.
   multimap<MC2String, MC2String> underviewByOverview;
   mc2dbg << " Listing map hierarchy" << endl;
   for ( map< MC2String, MC2String>::const_iterator theIt = 
            overviewByUnderview.begin();        
         theIt != overviewByUnderview.end();
         ++theIt ) 
   {
      MC2String underview = theIt->first;
      MC2String overview = theIt->second;
      underviewByOverview.insert(pair<MC2String,MC2String>(overview, 
                                                           underview));
      mc2dbg << "   " << overview << " is overview map to "
             << underview << endl;
   }
    
 

   // Set with the names of the country maps that are created here
   typedef set<MC2String> comapMap_t;
   comapMap_t countryMaps;

   // Map with wateritems added to the different country overview maps.
   // Key is map id. Value is item id for the water/forest items that 
   // are added to the country maps from that map.
   multimap<uint32, uint32> addedWaterIDs;
   multimap<uint32, uint32> addedForestIDs;
   


   // Go through the overview maps and process their underview maps.
   for ( vector<MC2String>::iterator overviewIt = overviewOrder.begin();
               overviewIt != overviewOrder.end(); ++overviewIt )
   {
      MC2String overview = *overviewIt;
      GMSCountryOverviewMap* curCountryMap = NULL;
      mc2dbg << "Overview: " << overview << endl;


      // This code makes it possible to ony generate one overview at a time
      // avoiding memory problems.
      if (oneMapAtATime){
         // Find first underview map of this overview map to determine
         // if the map has already been created.
         MC2String overviewName = *overviewIt;
         mc2log << info << "Processing country overview map " 
                << overviewName << endl;

         multimap<MC2String,MC2String>::const_iterator it =
            underviewByOverview.lower_bound(overviewName);

         uint32 firstUnderviewId = MAX_UINT32;
         bool firstUnderviewHdl = false;
         while ( it != underviewByOverview.upper_bound(overviewName) && 
                 firstUnderviewId == MAX_UINT32 )
         {
            // Loop the maps until you find an underview map actually
            // present because maybe we are only generating a subset of
            // the maps normally included in this overview map.
            MC2String underviewName = it->second;
            OldMapIdByName::mapIdAndHdlByName_t::const_iterator undIdIt =
               undIdAndHdlByName.find(underviewName);
            if ( undIdIt != undIdAndHdlByName.end() ){
               firstUnderviewId = undIdIt->second.first;
               firstUnderviewHdl = undIdIt->second.second;
            }
            ++it;
         }
         if (firstUnderviewId == MAX_UINT32){
            // Could not find the first map of this overview.
            mc2log << warn << "Could not find the first map of the"
                   << "overview: " << overviewName << endl;
            continue;
         }
         mc2dbg8 << firstUnderviewHdl << endl;
         if ( firstUnderviewHdl ){
            // This overview map has already been created, continue with 
            // next one.
            mc2log << info << "Already processed, continue." << endl;
            continue;
         }
      }

     
      // Go through underview maps of this overview map.
      multimap< MC2String, MC2String>::const_iterator it =
         underviewByOverview.lower_bound(overview);
      while (it != underviewByOverview.upper_bound(overview) ){
         MC2String underview = it->second;
         
         mc2dbg << "Underview: " << underview << endl;

         OldMapIdByName::mapIdAndHdlByName_t::iterator idAndHdlNameIt =
            undIdAndHdlByName.find(underview);
         if ( idAndHdlNameIt != undIdAndHdlByName.end()){
            // The underview map exists.

            uint32 mapId = 
               idAndHdlNameIt->second.first;// Id of this underview map.
            OldGenericMap* curMap = OldGenericMap::createMap(mapId, mapPath);
            mc2log << info << "Processing map " << mapId 
                   << " of country overview:" << overview << endl;

            
            comapMap_t::iterator coIt = countryMaps.find( overview );
            if (coIt == countryMaps.end()) {
               // No overview map found with this country code.
               // Create it.

               if ( curCountryMap != NULL ){
                  mc2log << error << "Already created current map: " << endl;
                  mc2log << error << curCountryMap->getMapName() 
                         << " when trying to create new one:" 
                         << overview << " Exits!" << endl;
                  exit(1);
               }
         
               // Did not find any country, create a new with next 
               // country map ID available in countryMapPath directory.
               uint32 nextCoMapID = 
                  MapGenUtil::getNextCoOvrID(countryMapPath);
               curCountryMap = 
                  new GMSCountryOverviewMap( nextCoMapID, countryMapPath );
               curCountryMap->createAllocators();
               curCountryMap->setCountryCode(curMap->getCountryCode());
               mc2dbg1 << "Setting country map name for " 
                    << curCountryMap->getMapID() << " to " 
                    << overview.c_str() << endl;
               curCountryMap->setCopyrightString( 
                                     curMap->getCopyrightString() );
               curCountryMap->setMapName( overview.c_str() );
               curCountryMap->setMapOrigin( curMap->getMapOrigin() );
               countryMaps.insert(overview);
            }

            // Add data from this map to this country
            if (curCountryMap->addDataFromMap(curMap)) {
               mc2log << info << "Added data from map " << curMap->getMapID();
            } else {
               mc2log << error << "FAILED to add data from map " 
                      << curMap->getMapID() << " Exits!" << endl;
               exit(1);
            }
            mc2log << " to country with ID " 
                   << curCountryMap->getMapID() << endl;

            
            // Done with this underview  map, delete it.
            delete curMap;

            idAndHdlNameIt->second.second = true; // Handled this map.
            // Update handled status on disk
            bool result = 
               mapIDsByName.writeFile();
            if ( !result ){
               mc2log << error << "GMS::createCountryMaps: " 
                      << "Failed to write OldMapIdByName::writeFile, exits!" 
                      << endl;
               exit(1);
            }

         }
         else{
            mc2log << info << "OldMap " << underview << " of overview map "
                   << overview << " not present!." << endl;
         }

         mc2dbg << "Increase it" << endl;
         it++; // Go to next underview map.

      }

      // Do some stuff with the overview map before changing to the 
      // next overview map.
      // 
      // NB! Do not use it, it points to the next map.


      // Add the water-items to the country-maps
      uint32 nbrAdded = curCountryMap->addWaterItems(addedWaterIDs);
      mc2log << info  << "Added " << nbrAdded << " water items to map " 
              << overview << endl;

      // Remove unwanted waters from the country overview map.
      // Already done in mixed post processing, so this call will do nothing
      uint32 nbrRemoved = curCountryMap->rmUnusedWater();
      mc2log << info << "Removed " << nbrRemoved << " unwanted water items "
             << "from the country overview map." << endl;

      // Add the forest-items to the country-maps
      nbrAdded = curCountryMap->addForestItems(addedForestIDs);
      mc2log << info  << "Added " << nbrAdded << " forest items to map " 
              << overview << endl;

      // Remove water-items and forest-item 
      // from the underview maps that were added to the country-maps.
      mc2log << info << "Removes water/forest items that were added to the "
             << "country overview map:" << overview << endl;
      // Go through underview maps of this overview map.
      it = underviewByOverview.lower_bound(overview);
      while (it != underviewByOverview.upper_bound(overview) ){
         MC2String underview = it->second;
         
         mc2dbg << "Underview: " << underview << endl;
         OldMapIdByName::mapIdAndHdlByName_t::iterator idAndHdlByNameIt =
            undIdAndHdlByName.find(underview);
         if ( idAndHdlByNameIt == undIdAndHdlByName.end()){
            mc2log << warn << "GMS::createCountryMaps "
                   << "Underview map in co.xml:" << underview 
                   << " of country overview"
                   << " map " << overview << " does not exist." 
                   << endl;
            // This is OK if we only generates a sub set of the maps
            // in the co.xml file.
            
         }
         else {
            // This underview map exists on disc.
               
            // Count removed water/forest items.
            uint32 removedWaterItems = 0;
            uint32 removedForestItems = 0;
            
            uint32 mapID = 
               idAndHdlByNameIt->second.first;// Id of this underview map. 
            bool addedWaters = 
               ( (addedWaterIDs.equal_range(mapID)).first  != 
                 (addedWaterIDs.equal_range(mapID)).second );
            bool addedForests = 
               ( (addedForestIDs.equal_range(mapID)).first  != 
                 (addedForestIDs.equal_range(mapID)).second );

            if ( addedWaters || addedForests ){
               // No need to load and save the map if there are no
               // water or forest items to remove.
               
               OldGenericMap* curMap = OldGenericMap::createMap(mapID, 
                                                          mapPath);
               if (curMap == NULL) {
                  mc2log << error << "GMS::createCountryMaps"
                         << " Could not open map with ID:0x" << hex  
                         << mapID << dec << " Exits!" << endl;
                  exit(1);
               }
               // Loop through and remove the added water items.
               mc2log << info << "GMS::createCountryMaps "
                      << "Removing water from:" << underview << endl;
               for ( multimap<uint32, uint32>::iterator it = 
                           (addedWaterIDs.equal_range(mapID)).first;
                     it != (addedWaterIDs.equal_range(mapID)).second;
                     ++it ){
                  // Remove
                  mc2dbg1 << "Removing water item " << it->second
                          << " from map 0x " << hex << mapID << dec 
                          << endl;
                  if (! curMap->removeItem(it->second)) {
                     mc2log << error << here 
                            << " Could not remove water item ("
                            << mapID << ", " << it->second << ")" 
                            << " Exits!" << endl;
                     exit(1);
                  }
                  removedWaterItems++;
               }  // for (each water item)
               // Loop through and remove the added forest items.
               mc2log << info << "GMS::createCountryMaps "
                      << "Removing forest from:" << underview << endl;
               for ( multimap<uint32, uint32>::iterator it = 
                           (addedForestIDs.equal_range(mapID)).first;
                     it != (addedForestIDs.equal_range(mapID)).second;
                     ++it ){
                  // Remove
                  mc2dbg1 << "Removing forest item " << it->second
                          << " from map 0x " << hex << mapID << dec 
                          << endl;
                  if (! curMap->removeItem(it->second)) {
                     mc2log << error << here 
                            << " Could not remove forest item ("
                            << mapID << ", " << it->second << ")" 
                            << " Exits!" << endl;
                     exit(1);
                  }
                  removedForestItems++;
               }  // for (each forest item)
               // Save map.
               curMap->save();
               // Delete the map to avoid memory leaks
               delete curMap;
            }
            mc2log << info << "Removed " << removedWaterItems 
                   << " waters and " << removedForestItems 
                   << " forest items from map:0x" << hex << mapID
                   << endl;
         }
         ++it; // Go to next underview map of this country overview.
      }  // while
      mc2log << info << "Done removing water/forest items "
             << "added to the country overvew map." << endl;
   



      // Make unique names in current country map.
      curCountryMap->updateGroupIDs();
      curCountryMap->makeUniqueNames();

      createCountryMapBorder( coh, curCountryMap );

           
      mc2log << info << "Merging streets in countrymap." << endl;
      curCountryMap->mergeStreetSegments();

      // Merge water items
      mc2log << info << "Merging waters in countrymap." << endl;
      GMSPolyUtility::mergeWaterItemsInCOMap( curCountryMap );
      // Merge forest items. 
      // NO, will take too long time with the available method, skip for now
      //mc2log << info << "Merging forests in countrymap." << endl;

      mc2log << info << " About to save countrymap "
             << curCountryMap->getMapName() << endl;

      if (!curCountryMap->save()) {
         mc2log << error << here << " Could not save country map"
                << "   mapID=" << curCountryMap->getMapID() 
                << " Exits!" << endl;
         exit(1);
      } else {
         mc2log << info <<  "Country map saved, mapID=" 
                 << curCountryMap->getMapID() << endl;
      }
      delete curCountryMap;


      if ( oneMapAtATime ){
         // We only wanted to create one map.
         mc2log << info << "One map handled, exits nicely" << endl;
         exit(0);
      }
   }

   mc2log << info << "All " << countryMaps.size()
           << " country maps created, " << endl;



   if ( oneMapAtATime ){
      // If we get here, no map was generated. Tell the script by sending
      // an exitcode.
      uint32 exitCode = 77;
      mc2log << info << "Found no map to process. exits with exit code:" 
             << exitCode << " to tell the script to stop overview " 
             << "generation." << endl;
      exit(exitCode);
   }
}

void createBorderItemsInCoMaps( const char* mapPath )
{
   // Loop co maps and create border items using
   //    - break points file CL_countryPolygonBreakPoints
   //    - borderItems.txt
   
   // Find first co map id
   uint32 curCoMapID = MapGenUtil::firstExistingCoOvrMapID(mapPath);
   bool cont = true;
   while ( cont ) {
      GMSCountryOverviewMap* curCountryMap = static_cast<GMSCountryOverviewMap*>
         ( GMSMap::createMap(curCoMapID, mapPath) );
      if ( curCountryMap != NULL ) {
      
         uint32 nbrBorderItems =
            createBorderItemsInOneCoMap( curCountryMap );
         if ( nbrBorderItems > 0 ) {
            curCountryMap->save();
         }
         delete curCountryMap;

         curCoMapID = MapBits::nextMapID(curCoMapID);
         
      } else {
         cont = false;
      }
   }
}

bool createBorderItemsInOneCoMap( GMSCountryOverviewMap* coMap )
{
   uint32 nbrBorderItems = 0;
   if (coMap != NULL) {
      nbrBorderItems = 
         coMap->createBorderItems( CL_countryPolygonBreakPoints );
      mc2log << info << "Created " << nbrBorderItems  
             << " border items in co map " << coMap->getMapName()
             << endl;
   }
   return nbrBorderItems;
}

bool filterCoPolCoordinateLevels(
               OldGenericMap* theMap, const char* breakPointsFileName )
{
   bool retVal = false;
   
   // get any country polygon break points (= coordinates defining
   // shared polygon parts (common lines) between neighbouring countries)
   set<MC2Coordinate> breakPoints;
   if ( breakPointsFileName != NULL ) {
      
      GMSUtility::extractCoPolBreakPoints( 
            breakPointsFileName, breakPoints );
      mc2log << info << "Extracted " << breakPoints.size()
             << " country polygon break points." << endl;
   } else {
      mc2log << error << here << "No break point coordinates file given"
             << endl;
      exit(1);
   }
   
   // filter
   if ( GMSUtility::filterCountryPolygonLevels(theMap, breakPoints) ) {
      retVal = true;
   }

   return retVal;
}

bool createGfxFilteringForCountryPolygons()
{
   mc2log << info << "Create gfx filtering (simplified gfx representation)"
          << " for co maps in this directory" << endl;
   
   const char* countryMapPath = "./";
   uint32 coMapID = FIRST_COUNTRYMAP_ID;
   
   uint32 nbrMaps = 0;
   bool cont = true;
   while (cont) {
      GMSCountryOverviewMap* coMap = dynamic_cast<GMSCountryOverviewMap*>
                              (GMSMap::createMap(coMapID, countryMapPath));
      if (coMap != NULL) {

         if ( createGfxFilteringForOneCountryPolygon(coMap) ) {
            coMap->save();
            nbrMaps++;
         }
         coMapID = MapBits::nextMapID(coMapID);
         
      } else {
         cont = false;
      }
      delete coMap;
   }

   mc2log << info 
          << "Created gfx filtering (simplified gfx representation) for " 
          << nbrMaps << " co maps" << endl;
   return true;
}

bool createGfxFilteringForOneCountryPolygon( GMSCountryOverviewMap* coMap )
{
   bool result = false;
   if (coMap != NULL) {
      if ( ! coMap->makeSimplifiedCountryGfx() ) {
         mc2log << error << "Coud not create gfx filtering of map gfx data"
                << " for map 0x" << hex << coMap->getMapID() << dec
                << " " << coMap->getMapName() << endl;
         return false;
      } else {
         mc2log << info << "Created gfx filterings for map 0x"
                << hex << coMap->getMapID() << dec << " " 
                << coMap->getMapName() << endl;
         result = true;
      }

   }
   return result;
}

/**
 *    Adds extra items from external maps (0x2001...).
 */
bool addExtraItems( CommandlineOptionHandler* coh )
{

   // Contains the maps with the extra items.
   vector<OldGenericMap*> extraItemMaps;

   const char* mapPath = "./"; // Hardcoded
   
   bool useTail = false;
   if ( coh->getTailLength() > 1 ) {
      // The first map in the tail is the map containing the extra items.
      uint32 id = (uint32) strtol( coh->getTail( 0 ), (char**)NULL, 10);
      OldGenericMap* otherMap = OldGenericMap::createMap( id, mapPath );
      if ( otherMap != NULL ) {
         extraItemMaps.push_back( otherMap );
         useTail = true;
      } else {
         mc2log << error << "Could not load the extraitem map " 
                << coh->getTail( 0 ) << endl;
         return false;
      }
   }
   
   // Load extra items maps into the map array.
   // The extra item maps has map id getExtItemFirstMapID and higher
   bool cont = true;
   uint32 tmpID = 0;
   while ( cont ) {
      OldGenericMap* otherMap =
         OldGenericMap::createMap(MapGenUtil::getExtItemFirstMapID()+tmpID,
                                  mapPath);
      if ( otherMap == NULL ) {
         cont = false;
      } else {
         extraItemMaps.push_back( otherMap );
         tmpID = MapBits::nextMapID(tmpID);
      }
   }

   if ( extraItemMaps.size() == 0 ) {
      mc2dbg << "No extra item maps" << endl;
      return true;
   }

   uint32 nbrBefore, nbrAfter;
   uint32 curMapID = 0;
   cont = true;
   set<IDPair_t> addedIDs;
   while (cont) {

      GMSMap* curMap = NULL;
      curMap = static_cast<GMSMap*>
                     (GMSMap::createMap(curMapID, mapPath));
      
      if (curMap != NULL) {
         mc2dbg1 << "Adding extra items in map " << curMap->getMapID() << endl;

         bool somethingAdded = false;
         for ( vector<OldGenericMap*>::iterator it = extraItemMaps.begin();
               it != extraItemMaps.end(); ++it ) {
            
            OldGenericMap* extraItemMap = *it;
            
            if (extraItemMap != NULL) {
               // Add streetsegments from otherMap
               nbrBefore = curMap->getNbrItems();
               
               // Add everything but municipals.
               curMap->addExtraItemsFromMap(
                                 extraItemMap, 
                                 addedIDs,
                                 false /* addMunicipals */ );
                     
               nbrAfter = curMap->getNbrItems();
               mc2dbg1 << "Added " << nbrAfter-nbrBefore 
                    << " items in " << curMap->getMapID() 
                    << " from " << extraItemMap->getMapID() << endl;
               if ( ! somethingAdded ) {
                  somethingAdded = ( (nbrAfter-nbrBefore) > 0 );
               }
            }
         }

         // Save this map to disk
         if (somethingAdded) {
            curMap->save();
         }

         // Increase the mapID-variable
         curMapID++;
      } else {
         cont = false;
      }

      // Delete the map to avoid memory leaks
      delete curMap;
   } // while
   
   // Delete the extraitem maps
   for ( vector<OldGenericMap*>::iterator it = extraItemMaps.begin();
         it != extraItemMaps.end(); ++it ) {
      delete *it;
   }

   return true;
}

bool readExtraData()
{
   //
   // Loop over all the maps in current directory
   // and add or check extra data.
   //---------------------------------------------------------------
   cout << "Read extra data: " << CL_readExtraData << endl;
   ifstream test(CL_readExtraData);
   if(test) {
      // Ok, extra data file exists
      // Check that the file is not empty.
      vector<MC2String> recordAsStrings;
      OldExtraDataUtility::readNextRecordFromFile(test, recordAsStrings);
      test.close();
      
      if ((recordAsStrings.size() == 0) ||
          (recordAsStrings[0].size() == 0)) {
         mc2log << "The extradata file " << CL_readExtraData
                << " is empty" << endl;
         CL_readExtraData = NULL;
         return false;
      }
      
      if(CL_verboseLevel > 1) {
         if (!CL_checkExtraData) {
            cout << "Adding extra data";
            if (CL_applyExtraDataOnMergedMaps)
               cout << " to merged maps";
            cout << endl;
         } else {
            cout << "Checking extra data" << endl;
         }
      }
      
      ExtraDataReader edr(CL_readExtraData);
      bool status = true;
      // If check, init the checkRecordTable in edr
      if (CL_checkExtraData) {
         status = edr.initCheckTable();
         if (!status) {
            cout << "Checking extra data, init check table not ok" << endl;
            CL_readExtraData = NULL;
            return false;
         }
      }
      // Get the time to use when updating dynamic ed time in the maps
      uint32 edTime = TimeUtility::getRealTime();
      time_t timeEdTime = time_t(edTime);
      // If dynamic ed, init
      if (CL_applyExtraDataOnMergedMaps) {
         edr.initMergedMaps();
      }

      // Add or check the extra data 
      const char* mapPath = "./";
      uint32 totNbrRecordsAdded = 0;
      uint32 nbrRecordsInFile = 0;
      bool allRecordsAdded = false;
      for (uint32 mapID = CL_startAtMap; mapID <= CL_endAtMap; mapID++) {
      
         if ( allRecordsAdded && !CL_usingDatabase ) {
            // ok to stop looping mcm maps
            mc2log << info << "GMSEDR All " << totNbrRecordsAdded
                   << " ed records added,"
                   << " break before loading map " << mapID << endl;
            break;
         }
         GMSMap* curMap = static_cast<GMSMap*>
                             (GMSMap::createMap(mapID, mapPath));
         if (curMap == NULL) {
            if ( !allRecordsAdded ) {
               mc2log << warn << "GMSEDR Only added " << totNbrRecordsAdded 
                      << " of " << nbrRecordsInFile
                      << " records to the looped mcm maps" << endl;
               if ( ! CL_checkExtraData ) {
                  // Print the ids of the records not added. Loop all line
                  // numbers from 0 to nbrRecordsInFile and check linear
                  // if the line nbr is present among the recordsAddedToMaps
                  vector<int> recordsAddedToMaps = 
                     edr.getRecordsAddedToMaps();
                  for (int line = 1; line <= int(nbrRecordsInFile); line++ ) {
                     bool found = false;
                     vector<int>::const_iterator iter =
                                    recordsAddedToMaps.begin();
                     while (!found && iter != recordsAddedToMaps.end() ) {
                        if ( *iter == line ) {
                           found = true;
                        }
                        iter++;
                     }
                     if ( ! found ) {
                        cout << " not added ed line " << line << endl;
                     }
                  }
               }
            }
            break;
         } else {
            if (CL_verboseLevel > 2) {
               cout << "\t" << setw(6) << mapID << endl;
            }
            edr.parseInto(curMap); // add or check the extradata
            
            nbrRecordsInFile = edr.getNbrRecordsInEDFile();
            uint32 nbrRecordsAdded = edr.getNbrRecordsAdded();
            totNbrRecordsAdded += nbrRecordsAdded;

            // Do we save the map? Any extra things to do?
            bool save = false;
            if ( CL_checkExtraData ) {
               // Check ed, never save
               save = false;
            } else if ( CL_applyExtraDataOnMergedMaps ) {
               // Dynamic ed, save only if something changed
               if ( edr.getItemsAffectedInUVMaps().count(mapID) > 0 ) {
                  save = true;
                  // Update the dynamic ed-time in the map
                  mc2dbg << "GMEDR Dynamic: added " << nbrRecordsAdded
                         << " records to map " << mapID << " -> affected " 
                         << edr.getItemsAffectedInUVMaps().count(mapID)
                         << " items, set dynamic extra data time to " << edTime
                         << " = " << asctime(localtime(&timeEdTime));
                  curMap->setDynamicExtradataTime(edTime);
                  // Update admin area centres
                  curMap->updateAdminAreaCentres();
               }
            } else {
               // Normal ed, save if something changed or if we must set
               // the dynamic ed-time running with CL_usingDatabase option
               mc2log << info << "GMSEDR Normal: added " << nbrRecordsAdded
                    << " records to map " << mapID << " (totally "
                    << totNbrRecordsAdded << " of " << nbrRecordsInFile
                    << ")" << endl;
               if ( nbrRecordsAdded > 0 ) {
                  save = true;
               }
               if ( CL_usingDatabase &&
                    (curMap->getDynamicExtradataTime() == MAX_UINT32) ) {
                  // Adding ED from DB & no dynamic ed-time defined, pls set.
                  // To handle that we re-use old map generation 
                  // after_mapDataExtr backups to
                  // start generation from, so the extradata time really
                  // matches the first extradata additions (and not using
                  // true creation time which then may be very old).
                  save = true;
                  mc2dbg << "GMSEDR Normal: set dynamic extra data time to "
                         << edTime
                         << " = " << asctime(localtime(&timeEdTime));
                  curMap->setDynamicExtradataTime(edTime);
               }
            }
            if (save) {
               curMap->save();
            }
            if ( totNbrRecordsAdded == nbrRecordsInFile ) {
               allRecordsAdded = true;
            }
         }
         delete curMap;
      }

      // If check, print the checkRecordTable
      if (CL_checkExtraData) {
         edr.printCheckTable();
      }

      // Write info to modifiedItemsED.txt to prepare for next step which 
      // must be to apply the extra data changes to overview and country 
      // overview maps.
      if (CL_applyExtraDataOnMergedMaps) {
         // Write modified map and item IDs.
         multimap<uint32, uint32> changedUnderviewIDs = 
            edr.getItemsAffectedInUVMaps();
         ofstream outFile ("modifiedItemsED.txt");
         for ( multimap<uint32, uint32>::const_iterator it =
                  changedUnderviewIDs.begin();
               it != changedUnderviewIDs.end();
               ++it ){
            uint32 mapID = it->first;
            uint32 itemID = it->second;
            outFile << mapID << ";" << itemID << endl;
         }
         outFile.close();
         
         // Loop the co maps and apply the extra data file again to make
         // corrections for all water items that were moved from the 
         // underview maps to only exist in co maps.

         // Problem: EDR takes a GMSMap as inparam

      }
   }
   else {
      cerr << "Cannot open extra data file '"
           << CL_readExtraData << "'" << endl;
      exit(1);
   }
   CL_readExtraData = NULL;

   return true;
}


// Fixme: change the name of this method:
// Used for changes from dynamic WASPing (POIS) modifiedPOIs.txt
// But also used for changes from dynamic extradata (map correction 
// records) modifiedItems.txt
bool
applyWASPChangesOnMergedMaps(const char* filename)
{
   // Read the file and store into a multimap<uint32, uint32> container.
   // Then call the applyChangesFromUnderviewsToOverviews-method.

   mc2dbg1 << "Apply changes from underview to overviews from "
           << filename << endl;
   ifstream infile(filename);
   if(infile) {
      // Ok, WASP file exists
      multimap<uint32, uint32> affectedItems;
      
      uint32 mapId, itemId;
      char t[1024];
      bool done = infile.eof();
      while ( !done ) {
         mapId = MAX_UINT32;
         itemId = MAX_UINT32;

         // mapid and itemid are separated by semicolon
         if (infile.getline(t, 1024, ';')) {
            mapId = strtoul(t, NULL, 10);
            infile >> itemId;
      
            mc2dbg4 << "Read mapId=" << mapId << " itemId=" << itemId << endl;
            if ((mapId != MAX_UINT32) && (itemId != MAX_UINT32))
               affectedItems.insert (pair<uint32, uint32>(mapId, itemId));
         }
            
         done = infile.eof();
      }

      mc2dbg2 << "Nbr items affected by WASP changes " << affectedItems.size()
              << " - applying to overview maps" << endl;
      applyChangesFromUnderviewsToOverviews(affectedItems);
      
      infile.close();
   }
   else {
      mc2log << error << "Missing modified items file: " << filename 
             << endl;
   }

   CL_applyWaspOnMergedMaps = NULL;
   
   return true;
}

bool
applyChangesFromUnderviewsToOverviews(
      multimap<uint32, uint32> itemsAffectedInUVMaps)
{
   mc2dbg << "GMS:applyChangesFromUnderviewsToOverviews, nbr affected items = "
          << itemsAffectedInUVMaps.size() << endl;
   if (itemsAffectedInUVMaps.size() == 0)
      return false;

   // Function that parses a table of affected items in underview maps
   // to overview and country overview maps. The changes originate either
   // from applying extra data or WASP-changes to production maps 
   // (merged maps).
   // 
   // 1. Extract which underveiw maps that have been changed, and find 
   //    corresponding overview- and country overview maps.
   //    (the multimap is "sorted" on underview id)
   //
   // 2. Loop the o- and co- maps and call functions in them to apply
   //    changes from resp underview map, copying attributes.
   //    (break the loop when all changes have been applied, even if
   //     there are more maps to process)
   // 
   // 3. Apply changes to super overview maps
   //    if only POIs changed, nothing will affect the super overview
   //
  
   
   vector<uint32> changedCOMaps;
   vector<uint32> changedOMapsFirstLevel;
   multimap<uint32,uint32> changedOverviewIdsFirstLevel;
   set<ItemTypes::itemType> changedOverviewFirstLevelItemTypes;
   bool changedOverviewFirstLevelNullItem = false;
   
   vector<uint32> changedUVids;
   multimap<uint32, uint32>::const_iterator cit;
   
   uint32 nbrUVChangesAppliedToOverviews = 0;
   bool allChangesFromUVApplied = false;
   
   const char* mapPath = "./";
   uint32 coMapID = MapGenUtil::firstExistingCoOvrMapID(mapPath);
   //uint32 coMapID = FIRST_COUNTRYMAP_ID;
   mc2dbg2 << "GMS:aCFUTO, trying comap 0x" << hex << coMapID << dec << endl;
   GMSCountryOverviewMap* curCOMap = dynamic_cast<GMSCountryOverviewMap*>
                           (GMSMap::createMap(coMapID, mapPath));
   while (!allChangesFromUVApplied && (curCOMap != NULL)) {

      // load the overview map corresponding to co-map
      uint32 overviewMapID = MapBits::countryToOverview(coMapID);
      OldOverviewMap* curOverviewMap = dynamic_cast<OldOverviewMap*>
                           (GMSMap::createMap(overviewMapID, mapPath));
      if (curOverviewMap == NULL) {
         mc2log << error << here << "Could not create overview map 0x"
                << hex << overviewMapID << " for country overview 0x"
                << coMapID << dec << endl;
         //break; ??
      }
      // To save co- and/or o-map after applying changes?
      bool saveCOMap = false;
      bool saveOMap = false;
      
      uint32 nbrMapsInCO = curCOMap->getNbrMaps();
      const char* countryName = StringTable::getString(
               StringTable::getCountryStringCode(curCOMap->getCountryCode()),
               StringTable::ENGLISH);
      cout << "GMS:aCFUTO, nbr maps in country " << countryName 
           << " = " << nbrMapsInCO << endl;
      
      // Loop the underview maps of this country overview...
      for (uint32 uvMapNbr = 0; uvMapNbr < nbrMapsInCO; uvMapNbr++) {

         uint32 creationTime;
         int32 maxLat, minLon, minLat, maxLon;
         uint32 curUVMapID = curCOMap->getMapData(
               uvMapNbr, creationTime, maxLat, minLon, minLat, maxLon);
         cout << "GMS:aCFUTO, uvMap " << uvMapNbr << " in country "
              << countryName << " is uvId " << curUVMapID << endl;

         // Find the changes that have been made for curUVMapID
         changedUVids.clear();
         cit = itemsAffectedInUVMaps.lower_bound(curUVMapID);
         while ( cit != itemsAffectedInUVMaps.upper_bound(curUVMapID)) {
            changedUVids.push_back( cit->second );
            cit++;
         }
         if (changedUVids.size() > 0) {
            // There are changes made for underview,
            // apply the changes on the country overview map and overview map
            
            cout << "GMS:aCFUTO, nbr changes for underview " << curUVMapID
                 << " is " << changedUVids.size() << endl;

            GMSMap* curUVMap = 
               static_cast<GMSMap*>(GMSMap::createMap(curUVMapID, mapPath));
            if (curUVMap != NULL) {
               bool ovChanged;
               // apply on country overview
               curCOMap->applyChangesFromOtherMap(
                     curUVMap, changedUVids, ovChanged);
               if (ovChanged) {
                  saveCOMap = true;
               }
               // apply on overview level 1
               curOverviewMap->applyChangesFromOtherMap(curUVMap, 
                                                        changedUVids, 
                                                        1,  // map level
                                                        ovChanged, 
                                                        changedOverviewIdsFirstLevel);
               
               if (ovChanged) {
                  saveOMap = true;
               }

               delete curUVMap;
            }
         }
         nbrUVChangesAppliedToOverviews += changedUVids.size();
      }
      if (nbrUVChangesAppliedToOverviews == itemsAffectedInUVMaps.size()) {
         // All changes have been applied, no need to loop any more
         // co- and o-maps..
         allChangesFromUVApplied = true;
      }
      

      // Save o- and co- map if something changed
      if (saveCOMap) {
         curCOMap->save();
         changedCOMaps.push_back(coMapID);
      }
      delete curCOMap;
      if (saveOMap) {
         
         curOverviewMap->save();
         changedOMapsFirstLevel.push_back(overviewMapID);

         // Collect item types of the items affected
         // (if only POIs, no need to parse changes overview level2)
         for (cit = changedOverviewIdsFirstLevel.lower_bound(overviewMapID);
              cit != changedOverviewIdsFirstLevel.upper_bound(overviewMapID);
              cit++) {
            OldItem* item = curOverviewMap->itemLookup( cit->second );
            if ( item != NULL ) {
               changedOverviewFirstLevelItemTypes.insert( item->getItemType());
            } else {
               changedOverviewFirstLevelNullItem = true;
            }
         }
      }
      delete curOverviewMap;

      // Increase the mapID-variable
      coMapID = MapBits::nextMapID(coMapID);
      mc2dbg1 << "GMS:aCFUTO, trying comap 0x" << hex << coMapID << dec 
              << " (allChangesFromUVApplied = ";
      if (allChangesFromUVApplied)
         mc2dbg1 << "true -> break the loop!";
      else
         mc2dbg1 << "false";
      mc2dbg1 << ")" << endl;
      if (!allChangesFromUVApplied) {
         curCOMap = dynamic_cast<GMSCountryOverviewMap*>
                    (GMSMap::createMap(coMapID, mapPath));
      }
   }

   mc2log << info << "GMS:aCFUTO, applied " << nbrUVChangesAppliedToOverviews
          << " uv changes to " << changedCOMaps.size() 
          << " country overview maps" << endl;
   mc2log << info << "GMS:aCFUTO, applied " << nbrUVChangesAppliedToOverviews
          << " uv changes to " << changedOMapsFirstLevel.size()
          << " overview maps" << endl;
   mc2log << info << " - resulted in " << changedOverviewIdsFirstLevel.size() 
          << " overview level_1 changes" << endl;

   // Apply changes to Overview map level 2 (european overview)
   if (changedOMapsFirstLevel.size() == 0) {
      cout << "No changes to apply to overview map(s) level_2" << endl;
   } else if (changedOMapsFirstLevel.size() > 0) {
      cout << "To apply changes to overview map(s) level_2 due to totally "
           << changedOverviewIdsFirstLevel.size() << " changes in "
           << changedOMapsFirstLevel.size() << " overview level_1" << endl;
    
      cout << "Applying changes from these overview level_1 maps:" << endl;
      vector<uint32>::iterator it;
      for (it = changedOMapsFirstLevel.begin();
           it != changedOMapsFirstLevel.end(); it++) {
         uint32 ovId = (*it);
         cout << " overview map 0x0" << hex << ovId << dec << ":" << endl;
         uint32 j = 0;
         for (cit = changedOverviewIdsFirstLevel.lower_bound(ovId);
              cit != changedOverviewIdsFirstLevel.upper_bound(ovId); cit++) {
            j++;
            cout << " " << cit->second;
            if ((j % 6) == 0) // avoid having all ids in one row...
               cout << endl;
         }
         if ((j % 6) != 0)
            cout << endl;
      }

      // Only apply if the items affected in overview level1 are of
      // other item types than POIs
      bool onlyPOIs = false;
      if ( changedOverviewFirstLevelNullItem ) {
         // we don't know if the removed items are POIs or anything else
         cout << "changedOverviewFirstLevelNullItem true" << endl;
      }
      cout << "changedOverviewFirstLevelItemTypes.size=" 
           << changedOverviewFirstLevelItemTypes.size() << endl;
      if ( (changedOverviewFirstLevelItemTypes.size() == 1) &&
           (changedOverviewFirstLevelItemTypes.find(
               ItemTypes::pointOfInterestItem) != 
                  changedOverviewFirstLevelItemTypes.end()) ) {
         onlyPOIs = true;
      }
      
      // Create superoverview (level2)
      // Check which overviewmaps(level1) that have changed.
      // Create them and apply changes.
      
      OldOverviewMap* superOverviewMap = NULL;
      uint32 superOverviewMapId = 
         ((FIRST_OVERVIEWMAP_ID & 0xf0000000) + (1 << 28));
      if ( onlyPOIs ) {
         // only POIs affected in overview maps, no need to load level2
         mc2dbg << "Only POIs affected in the overview level 1 maps -> "
                << "no need to load overview level_2 maps" << endl;
      } else {
         mc2dbg1 << "Trying to load overview level_2 0x0" << hex 
                 << superOverviewMapId << dec << endl;
         superOverviewMap = dynamic_cast<OldOverviewMap*>
                           (GMSMap::createMap(superOverviewMapId, mapPath));
      }
      if (superOverviewMap == NULL) {
         mc2log << warn << "Could/did not create super overview map 0x0"
                << hex << superOverviewMapId << dec << endl;
      } else {
         
         bool saveOverviewLevel2 = false;
         multimap<uint32, uint32> changedSuperOVids;
         // Loop all overview maps level 1 that was affected by any change
         // to apply the changes to the super overview as well.
         vector<uint32>::iterator it;
         uint32 nbrChangesInSuperOV = 0;
         for (it = changedOMapsFirstLevel.begin();
              it != changedOMapsFirstLevel.end(); it++) {
            
            uint32 curOverviewMapId = (*it);
            
            // Find the changes that have been made for curOverviewMapId
            vector<uint32> changedOVids;
            cit = changedOverviewIdsFirstLevel.lower_bound(curOverviewMapId);
            while ( cit != changedOverviewIdsFirstLevel.upper_bound(
                                                curOverviewMapId)) {
               changedOVids.push_back( cit->second );
               cit++;
            }
            if (changedOVids.size() > 0) {
               // There are changes made for the overview level 1,
               // apply the changes on the overview map level 2
               
               cout << "GMS:aCFUTO, nbr changes for overview level_1 0x0" 
                    << hex << curOverviewMapId << dec << " is " 
                    << changedOVids.size() << endl;
               
               OldOverviewMap* curOverviewMap = dynamic_cast<OldOverviewMap*>
                     (GMSMap::createMap(curOverviewMapId, mapPath));
               if (curOverviewMap != NULL) {
                  bool ovChanged = false;
                  // apply to overview level 2
                  superOverviewMap->applyChangesFromOtherMap( curOverviewMap,
                        changedOVids, 2, ovChanged, changedSuperOVids);
                  mc2log << info << "Applying ov level_1 changes from map 0x0"
                         << hex << curOverviewMapId << dec << " resulted in " 
                         << (changedSuperOVids.size()-nbrChangesInSuperOV)
                         << " level_2 changes" << endl;
                  nbrChangesInSuperOV = changedSuperOVids.size();
                  if (ovChanged) {
                     saveOverviewLevel2 = true;
                  }

                  delete curOverviewMap;
               }
            }
         }
         mc2log << info << "Applied totally " << changedSuperOVids.size()
                << " changes to overview 0x0" << hex
                << superOverviewMapId << dec << endl;
         for (cit = changedSuperOVids.begin(); 
              cit != changedSuperOVids.end(); cit++) {
            cout << " changed 0x" << hex << cit->first << dec << "."
                 << cit->second << endl;
         }

         // Save the overview level 2 if any changes were made.
         if (saveOverviewLevel2) {
            superOverviewMap->save();
         }
         delete superOverviewMap;

         // Increase superOverviewMapId and try to load next
         // super ov map. If there are more than the first one, we need
         // to exit this function because it does not handle multiple
         // super overviews in one map merge directory!
         superOverviewMapId = MapBits::nextMapID(superOverviewMapId);
         superOverviewMap = dynamic_cast<OldOverviewMap*>
                           (GMSMap::createMap(superOverviewMapId, mapPath));
         if ( superOverviewMap != NULL ) {
            mc2log << error << here << "Loaded super overview 0x0"
                   << hex << superOverviewMapId << dec 
                   << " but this method does not handle "
                   << "multiple super overviews in one map directory!!!"
                   << endl;
            MC2_ASSERT(false);
            delete superOverviewMap;
         }
      }
      
   }
   
   return true;
} // applyChangesFromUnderviewsToOverviews

bool mixedPostProcessing(GMSMap* theMap){

   // ====================================================================
   // Now that everything is added to the map, it is time to do some
   // special stuff.

   // Set native languages and update official names in the map
   setNativeLanguages(theMap);
   
   // Removes groups that are items that should not be included in the search
   // index.
   mc2log << info << "[mixedPostProcessing] Removing non search index groups."
          << endl;
   theMap->removeNonSearchIdxGroups();

   // Remove unwanted waters from the map, i.e. waters that are not
   // used by the server. Do this before merging items/hole elimination
   uint32 nbrRemoved = theMap->rmUnusedWater();
   mc2log << info << "[mixedPostProcessing] Removed " << nbrRemoved
          << " unwanted water items from the map." << endl;

   // Remove zip codes duplicated by zip overviews in the overview map.
   uint32 minimumLength = 
      NationalProperties::getZipMinLength(theMap->getCountryCode());
   uint32 nbrZipsRemoved = theMap->removeShorterZipCodes(minimumLength);
   mc2log << info << " Removed " << nbrZipsRemoved 
          << " zips because they are shorter than " << minimumLength << endl;

   // Remove no-name city parts
   uint32 nbrRemovedCP = theMap->removeNoNameCityParts();
   mc2log << info << "[mixedPostProcessing] Removed " << nbrRemovedCP 
          << " no-name city parts" << endl;
   
   
   // Remove no-name buas that are located in no-name municipals
   if ( NationalProperties::removeNoNameBuasInNoNameMunicipals(
            theMap->getCountryCode(), theMap->getMapOrigin()) ) {
      uint32 nbrRemoved = theMap->removeNoNameBuasInNoNameMunicipals();
      mc2log << info << "[mixedPostProcessing] Removed " << nbrRemoved 
             << " no-name buas in no-name municipals" << endl;
   }

   // Remove identical and un-necessary coordinates from item gfxdata
   mc2log << info << "[mixedPostProcessing] "
          << "rmPolygonDefectsAndUnnecessaryCoords" << endl;
   uint32 nbrChanged = 
      GMSPolyUtility::rmPolygonDefectsAndUnnecessaryCoords(theMap);
   mc2log << info << "[mixedPostProcessing] Removed defects from "
          << nbrChanged << " item polygons in the map." << endl;

   // Make sure all municipals have a name and not the $$$ name
   // Call before mergeSameName municipals
   mc2log << info << "[mixedPostProcessing] updateMunicipalNames" << endl;
   uint32 nbrMunsWithNameUpdate = theMap->updateMunicipalNames();
   mc2log << info << "[mixedPostProcessing] updateMunicipalNames "
          << nbrMunsWithNameUpdate << " municipals with names updated"
          << endl;
   
   // Make sure all buas have a name
   mc2log << info << "[mixedPostProcessing] setNamesOnNoNameBuas" << endl;
   uint32 nbrBuas = theMap->setNamesOnNoNameBuas();
   mc2log << info << "[mixedPostProcessing] setNamesOnNoNameBuas "
          << nbrBuas << " noname buas got name" << endl;

   // Merge built-up areas that are split into several items
   // Update the builtUpAreaLocation of items in any merged bua.
   uint32 mergeDist = 
    NationalProperties::getAdminAreasMergeDist( theMap->getCountryCode() );
   mc2log << info << "[mixedPostProcessing] merge municipals" << endl;
   theMap->mergeSameNameAdminAreas( 
         ItemTypes::municipalItem, mergeDist, true );
   mc2log << info << "[mixedPostProcessing] merge BUAs" << endl;
   theMap->mergeSameNameAdminAreas( 
         ItemTypes::builtUpAreaItem, mergeDist, true );
   

   // Compact municipals
   // Includes the update of municipal array
   mc2log << info << "[mixedPostProcessing] compactMunicipals" << endl;
   theMap->compactMunicipals();
   

   // If zip codes from SSI attribute exists, all zip code areas with 
   // true geometry are deleted.
   // Else SSIs are added to zip codes with true geometry using gfx data.
   mc2log << info << "[mixedPostProcessing] addSSIToZipCodesUsingGfx" 
          << endl;
   theMap->addSSIToZipCodesUsingGfx();
   
   // Merge all the zips.
   mc2log << info << "[mixedPostProcessing] merge zip codes" << endl;   
   theMap->mergeSameNameAdminAreas( ItemTypes::zipCodeItem, MAX_UINT32, 
                                    false, false ); 
   
   // Set map gfx data
   if (theMap->setMapGfxDataConvexHull()) {
      mc2dbg2 << "GfxData convex hull set to theMap succesfully" << endl;
   } else {
      mc2log << error << "GfxData convex hull set to theMap unsuccesfully!"
             << endl;
   }

   // Set the logical location for all the items
   //      To do this the municipal- and BUA-GfxData must be closed!
   //      This is done temporary here, in case not given so from
   //      map supplier map data
   mc2log << info << "[mixedPostProcessing] set gfx data closed for all"
          << " BUAs and municipals" << endl;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z ) {
      for ( uint32 i = 0; i < theMap->getNbrItemsWithZoom( z ); ++i ) {
         OldItem* item = theMap->getItem( z, i );
         if ( ( item != NULL ) && ( item->getGfxData() != NULL ) ) {
            if ( item->getItemType() == ItemTypes::municipalItem )
               item->getGfxData()->setClosed(0,true);
            else if ( item->getItemType() == ItemTypes::builtUpAreaItem )
               item->getGfxData()->setClosed(0,true);
         }
      }
   }
   

   // Create the hashtable.
   mc2log << info << "[mixedPostProcessing] build hash table of map" 
          << endl;      
   theMap->buildHashTable();


   // Remove connections connecting nodes with different levels.
   mc2log << info << "[mixedPostProcessing] remove conns between nodes"
          << " on different levels." << endl;
   uint32 nbrRemovedConns = theMap->removeConnsWithDiffLevel();
   mc2dbg1 << nbrRemovedConns << " connections removed due to different levels"
           << endl;

   // Merge same-name close items, misc display items
   for ( uint32 i = 0; i < uint32(ItemTypes::numberOfItemTypes); i++ ) {
      if ( NationalProperties::mergeSameNameCloseItems(
            theMap->getCountryCode(), theMap->getMapOrigin(),
            ItemTypes::itemType(i), theMap->getMapID() ) ){
         mc2log << info << "[mixedPostProcessing] Merge "
                << "same name close items, type "
                << ItemTypes::getItemTypeAsString(
                     ItemTypes::itemType(i)) << endl;
         uint32 nbrMerged = 
            GMSPolyUtility::mergeSameNameCloseItems(
               theMap, ItemTypes::itemType(i), true ); // only closed polys
         mc2log << info << "[mixedPostProcessing] "
                << "mergeSameNameCloseItems Merged "
                << "same name close items for "
                << nbrMerged << " items" << endl;
      }
   }

   // Set location for all items that have invalid location.
   // Index areas have geomtery
   mc2log << info << "[mixedPostProcessing] setAllItemLocation" << endl;
   theMap->setAllItemLocation(true);
   

   // Remove empty muncipals
   mc2log << info << "[mixedPostProcessing] remove empty municipals" 
          << endl;
   theMap->removeEmptyLocationItems( ItemTypes::municipalItem );
   
   // Remove empty buas (i.e. the named areas that could not be set
   // as location).
   mc2log << info << "[mixedPostProcessing] remove empty BUAs" 
          << endl;
   theMap->removeEmptyLocationItems( ItemTypes::builtUpAreaItem );
   

   // Update the bua names so that they contain the same foreign
   // names as their municipals do.
   mc2log << info << "[mixedPostProcessing] Update BUA names from"
          << " municipals" << endl;
   uint32 nbrAddedNames = theMap->updateForeignBuaNames();
   mc2dbg << "Added " << nbrAddedNames << " foreign bua names." << endl;
  
   // Update speedlimits for cities with high levels of traffic congestion.
   // Currently London
   mc2log << info << "[mixedPostProcessing] updateCongestionSpeedLimits" 
          << endl;
   uint32 nbrAdjustedSpeeds = theMap->updateCongestionSpeedLimits();
   mc2dbg << "Updated speedlimits for " << nbrAdjustedSpeeds 
          << " roads in cities with congested traffic."
          << endl;
   
   
   // Removing exact duplicates of names of the same language on SSIs.
   mc2log << info << "[mixedPostProcessing] Removing exact duplicates"
          << " of names of the same language on SSIs." << endl;
   if ( ! theMap->removeDuplicatedStreetNames() ){
      mc2log << error << "mixedPostProcessing, "
             << "removeDuplicatedStreetNames failed. Exits!" << endl;
      exit(1);
   }


   // Remove non-numeric zip code names, i.e. all names that do not have
   // the name type ItemTypes::roadNumber
   if (NationalProperties::rmNonNumZipNames(theMap->getCountryCode())){
      mc2log << info << "[mixedPostProcessing] "
             << "Removing non-numeric zip code names." << endl;
      uint32 nbrItemsWithNamesRemoved = theMap->removeNonNumericZipCodeNames();
      mc2log << info << "[mixedPostProcessing] "
             << "Removed zip names from " << nbrItemsWithNamesRemoved 
             << " zip items." << endl;
   }

   // Merge polygons within items
   mc2log << info << "[mixedPostProcessing] Merge item polygons" << endl;
   uint32 nbrMerged = GMSPolyUtility::mergeItemPolygons( theMap );
   mc2log << info << "[mixedPostProcessing] Merged item polygons in " 
          << nbrMerged << " items" << endl;

   mc2log << info << "[mixedPostProcessing] Setting undefined lane dir cat" 
          << endl;
   uint32 nbrSetCat = theMap->setUndefiendLaneConnDirCat();
   mc2dbg << info << "[mixedPostProcessing] Nbr updated dir cat: " 
          << nbrSetCat << endl;

   // Eliminate holes
   // Be careful which item types you do this for. The method is slow for
   // large items with many holes. There will be small gaps between
   // the small resulting poly parts.

   // Some resulting info from eliminateHoles, is needed by eliminateSelfTouch.
   for ( uint32 i = 0; i < uint32(ItemTypes::numberOfItemTypes); i++ ) {
      if ( NationalProperties::eliminateHolesAndSelftouch(
            theMap->getCountryCode(), theMap->getMapOrigin(),
            ItemTypes::itemType(i) ) ){
      
         // Filled in elimHoles, used in elimSelftouch
         set<MC2Coordinate> coordsAddedToReverseLine;

         // Eliminate holes in polygons (follow by elimST)
         mc2log << info << "[mixedPostProcessing] Eliminate holes: "
                << ItemTypes::getItemTypeAsString(
                        ItemTypes::itemType(i)) << endl;
         GMSPolyUtility::eliminateHoles( theMap, 
                                         coordsAddedToReverseLine, 
                                         ItemTypes::itemType(i) );
         // Eliminate self-touching polygons (directly after elimHoles)
         mc2log << info << "[mixedPostProcessing] Eliminate self-touch: "
                << ItemTypes::getItemTypeAsString(
                        ItemTypes::itemType(i)) << endl;
         GMSPolyUtility::eliminateSelfTouch( theMap, 
                                             coordsAddedToReverseLine, 
                                             ItemTypes::itemType(i) );
      }
   }


   // Compute area feature draw display classes and print result
   mc2log << info << "[mixedPostProcessing] Compute area feature " 
          << "draw display classes" << endl;
   theMap->computeAreaFeatureDrawDisplayClasses(); 
   theMap->printAreaFeatureDrawDisplayClassInfo();
  

   // Note that street generation and turndescriptions must be run 
   // afterwards.
 
   return true;
} // mixedPostProcessing


bool
getExtradataTimePerCountry(const char* fileName)
{
   map<StringTable::countryCode, pair<uint32, MC2String> >
      datesAndMapOrigPerCountry;
   map<StringTable::countryCode, pair<uint32, MC2String> >::iterator it;

   const char* mapPath = "./";
   for (uint32 mapID = CL_startAtMap; mapID <= CL_endAtMap; mapID++) {
      GMSMap* curMap = static_cast<GMSMap*>
                          (GMSMap::createMap(mapID, mapPath));
      if (curMap == NULL) {
         break;
      } else {
         
         // Get the latest ed-time for this map, dynamicExtradataTime in 
         // first hand or in second hand origCreationTime
         uint32 latestEDdate = MAX_UINT32;
         if (curMap->getDynamicExtradataTime() != MAX_UINT32)
            latestEDdate = curMap->getDynamicExtradataTime();
         else
            latestEDdate = curMap->getTrueCreationTime();
         
         if (latestEDdate != MAX_UINT32) {
            StringTable::countryCode cCode = curMap->getCountryCode();

            // Store in datesAndMapOrigPerCountry if this country was not 
            //stored before, or the date is prior to the existing.
            it = datesAndMapOrigPerCountry.find(cCode);
            if (it == datesAndMapOrigPerCountry.end()) {
               pair<uint32, MC2String> dateAndOrigin = 
                  make_pair(latestEDdate,
                            MC2String( curMap->getMapOrigin() ));
               datesAndMapOrigPerCountry.insert(make_pair(cCode, 
                                                          dateAndOrigin) );
            } else if ((*it).second.first > latestEDdate) {
               (*it).second.first = latestEDdate;
            }

            time_t tmpLatestEDdate = static_cast<time_t>(latestEDdate);
            mc2dbg1 << StringTable::getString(
                           StringTable::getCountryStringCode(cCode),
                           StringTable::ENGLISH)
                    << " map " << curMap->getMapID() << " ed-date: "
                    << asctime(localtime( &tmpLatestEDdate ))
                    << "mapOrigin:" << curMap->getMapOrigin() << endl;
         }
      }
      delete curMap;
   }

   // Print the extradata dates to the outfile.
   ofstream outfile(fileName);
   
   // Get printable string for date, to be used as inparam to
   // the ExtradataExtractor.
   for (it = datesAndMapOrigPerCountry.begin(); 
        it != datesAndMapOrigPerCountry.end(); it++) {
      time_t edDate = (*it).second.first;
      char date[126];
      if ( sprintf(date,
                   "%04d-%02d-%02d %02d:%02d:%02d",
                   localtime(&edDate)->tm_year + 1900,
                   localtime(&edDate)->tm_mon + 1,
                   localtime(&edDate)->tm_mday,
                   localtime(&edDate)->tm_hour,
                   localtime(&edDate)->tm_min,
                   localtime(&edDate)->tm_sec) ) {
         StringTable::countryCode cCode = (*it).first;
         
         MC2String mapOrigin = (*it).second.second;
         mc2dbg1 << "Country " << int(cCode) << " "
                 << StringTable::getString(
                        StringTable::getCountryStringCode(cCode),
                        StringTable::ENGLISH)
                 << " has ed-date \"" << date << "\" and origin:"
                 << mapOrigin << endl;
         outfile << int(cCode) << ";"
                 << StringTable::getString(
                        StringTable::getCountryStringCode(cCode),
                        StringTable::ENGLISH)
                 << ";" << date << ";" << mapOrigin << endl;
      }
   }
   return true;
} // getExtradataTimePerCountry


bool setNativeLanguages( OldGenericMap* theMap ){
   bool result = true;

   // Fix the official names that should not be for the underview maps.
   if ( MapBits::isUnderviewMap( theMap->getMapID() ) ) {
      theMap->updateNativeLanguages();
      uint32 nbrChanged = theMap->updateOfficialNames();
      if ( nbrChanged != 0 ) {
         mc2log << info << "[OldGenericMap]: Changed " << nbrChanged
                << " official names into alternative" << endl;
      }
   }
   else {
      mc2log << error << "Trying to set native language for overview map."
             << " Exits!" << endl;
      exit(1);
   }
   return result; 
} // setNativeLanguages       


void createNewIndex(char* coXMLFilesStr, 
                    const char* cooXMLFilesStr)
{
   const char* mapPath = "./";

   // Initialize the XML system
   try {
      XMLPlatformUtils::Initialize();
   } 
   catch(const XMLException& toCatch) {
      cerr << "Error during Xerces-c Initialization.\n"
           << "  Exception message:"
           << toCatch.getMessage() << endl;
      exit(1);
   }


   // Check and fix in-paramters sanity.
   if ( coXMLFilesStr == NULL ){
      mc2dbg << "coXMLFilesStr == NULL" << endl;
   }
   else {
      mc2dbg << "coXMLFilesStr == \"" << coXMLFilesStr << "\""
             << endl;
   }
   if ( cooXMLFilesStr == NULL ){
      mc2dbg << "cooXMLFilesStr == NULL. Sets it to empty string"
             << endl;
      cooXMLFilesStr="";
   }
   else {
      mc2dbg << "cooXMLFilesStr == \"" << cooXMLFilesStr 
             << "\"" << endl;
   }
   if ( ( coXMLFilesStr == NULL ) ||
        ( strlen( coXMLFilesStr ) == 0 ) ||
        ( cooXMLFilesStr == NULL ) ){
      mc2log << error << "Option given: --createNewIndex." 
             << " Missing other options for xml files. Check -h"
             << " to see which additional options are mandatory"
             << " together with --createNewIndex." << endl;
      exit(1);
   }
   
   char token = ' ';    // File names are separated by space.
   vector< char* > coXMLFiles;
   StringUtility::tokenListToVector( coXMLFiles,
                                     coXMLFilesStr,
                                     token );
   vector< char* > cooXMLFiles; 
   StringUtility::tokenListToVector( cooXMLFiles,
                                     (char*)cooXMLFilesStr,
                                     token );
   mc2dbg << "(GMS) coXMLFilesStr = " << coXMLFilesStr << endl;
   mc2dbg << "(GMS) coXMLFiles->size() = " << coXMLFiles.size() << endl;
   mc2dbg << "(GMS) cooXMLFiles->size() = " << cooXMLFiles.size() << endl;
   
   mc2log << info << "Process overview by underview XML" << endl;
   // call only if coXMLFiles is set
   map<MC2String, MC2String> overviewByUnderview;
   if ( coXMLFiles.size() > 0 ) {
      const set<const char*> coXMLFilesChSet = 
         StringUtility::stringVectorToCharSet( coXMLFiles );
      overviewByUnderview =
         XMLIndata::parseCreateOverviewXMLData( coXMLFilesChSet );
   }
   
   mc2log << info << "Process super by overview XML" << endl;      
   // call only if cooXMLFiles is set
   map<MC2String, MC2String> superByOverview;
   if ( cooXMLFiles.size() > 0 ) {
      const set<const char*> cooXMLFilesChSet = 
         StringUtility::stringVectorToCharSet( cooXMLFiles );
      superByOverview =
         XMLIndata::parseCreateOverviewXMLData( cooXMLFilesChSet );
   }
   
   // Create the new index.db file.
   mc2log << info << "Creates index from maps." << endl;
   MapIndex mapIndex(mapPath);
   mapIndex.createFromMaps( overviewByUnderview, superByOverview );
   mc2log << info << "Saving index.db" << endl;
   mapIndex.save();
   
   // Shut down the XML system
   XMLPlatformUtils::Terminate();         
   
   mc2log << info << "New index created, exiting" << endl;
   exit(0);

} // createNewIndex


void listIndex() {
   // Create mapIndex object.
   const char* mapPath = "./";
   MapIndex mapIndex(mapPath);
   mapIndex.load();
   mapIndex.list();

} // listIndex

/**
 * @param arXMLFilesStr Should contain at least one *_ar.xml file and
 *        the region_ids.xml file.
 */
void addRegions(char* arXMLFilesStr, MapIndex& mapIndex){

   // Initialize the XML system
   try {
      XMLPlatformUtils::Initialize();
   } 
   catch(const XMLException& toCatch) {
      cerr << "Error during Xerces-c Initialization.\n"
           << "  Exception message:"
           << toCatch.getMessage() << endl;
      exit(1);
   }

   // Parse XML files in-parameter
   char token = ' ';    // File names are separated by space.
   vector<char*> arXMLFiles;
   StringUtility::tokenListToVector( arXMLFiles,
                                     arXMLFilesStr,
                                     token );

   const set<const char*> arXMLFilesChSet = 
      StringUtility::stringVectorToCharSet( arXMLFiles );

   XMLIndata::parseAndAddRegionsToIndex(mapIndex, arXMLFilesChSet);


   // Shut down the XML system
   XMLPlatformUtils::Terminate();         


} // addRegions

void updateCreationTimes(){
   const char* mapPath = "./";
   MapIndex mapIndex(mapPath);
   mapIndex.load();

   if (mapIndex.updateCreationTimes()){
      mc2log << info << "Saving index.db" << endl;
      mapIndex.save();
   }
   else {
      mc2log << info << "No changes to index.db" << endl;
   }

   mc2log << info << "Creation times updated, exiting" << endl;
   exit(0);
}

void createOverview(char* coXMLFilesStr)
{

   /* Concepts in this method explained.
    * 
    * In this method underview and lower map are equivalent, and 
    * overview and higher map are equivalent. The concepts are mixed below.
    *
    * When generating level 1 overview maps the higher maps are the 
    * overview maps and the lower maps are the underview maps. When 
    * generating level 2 overview maps, i.e. super overviews, the higher
    * maps are super overview maps and the lower maps are overview maps.
    */

   // Used for finding maps.
   const char* mapPath = "./";

   // Used in debug prints.
   const char* methodName = "GMS::createOverview: "; 

   // Set to false to create all overviews without exits.
   bool oneMapAtATime=true;

   // Initialize the XML system
   try {
      XMLPlatformUtils::Initialize();
   } 
   catch(const XMLException& toCatch) {
      cerr << "Error during Xerces-c Initialization.\n"
           << "  Exception message:"
           << toCatch.getMessage() << endl;
      exit(1);
   }

   // Check and fix in-paramters sanity.
   if ( coXMLFilesStr == NULL ){
      mc2dbg << "coXMLFilesStr == NULL" << endl;
   }
   else {
      mc2dbg << "coXMLFilesStr == \"" << coXMLFilesStr << "\""
             << endl;
   }
   if ( ( coXMLFilesStr == NULL ) ||
        ( strlen( coXMLFilesStr ) == 0 ) ){
      mc2log << error << "Option given: --createOverview." 
             << " Missing other options for xml files. Check -h"
             << " to see which additional options are mandatory"
             << " together with --createOveriew." << endl;
      exit(1);
   }
        
   char token = ' ';    // File names are separated by space.
   vector< char* > coXMLFiles;
   StringUtility::tokenListToVector( coXMLFiles,
                                     coXMLFilesStr,
                                     token );

   mc2dbg << methodName << "coXMLFilesStr = " << coXMLFilesStr
          << endl;
   mc2dbg << methodName << "coXMLFiles->size() = " << coXMLFiles.size()
          << endl;
   
   mc2log << info << "Process overview by underview XML" << endl;
   const set<const char*> coXMLFilesChSet = 
      StringUtility::stringVectorToCharSet( coXMLFiles );
   map<MC2String, MC2String> overviewByUnderview =
      XMLIndata::parseCreateOverviewXMLData( coXMLFilesChSet );

   // Debug print.
   mc2dbg << methodName << "Listing map hierarchy."
          << endl;
   for (map<MC2String, MC2String>::const_iterator 
         it(overviewByUnderview.begin());        
         it != overviewByUnderview.end(); ++it ) 
   {
      mc2dbg << "   " << it->second << " is overview map to "
             << it->first << endl;
   }
      
   // Maps map ID and handled status to map name. Handled status is true
   // if the map has been added to an overview map and false otherwise.
   MC2String mapIDsByNameFile = "mapName2MapID.txt";
   OldMapIdByName mapIDsByName( mapIDsByNameFile );
   bool recreateMapIDsByName = true;

   if ( mapIDsByName.fileExists() ){
      mc2log << info << "MM::createOverview" 
             << " OldMapping map ID to name file found, read it." << endl;

      recreateMapIDsByName = false;
      bool result = mapIDsByName.readFile();
      if ( !result ){
         mc2log << error << "MM::createOverview: " 
                << "Failed to read underview to map ID file."
                << " Exits!" << endl;
         exit(1);
      }

      // Check that all underview maps are present in the file.
      uint32 highestID = 0;
      uint32 nextMapID = 0;
      result = mapIDsByName.cmpUndIDsWithMapFiles( mapPath,
                                                   highestID,
                                                   nextMapID );
      if ( ! result ){
         mc2log << error << "MM::createOverview: " << hex 
                << "(highestID+1) != nextMapID"
                << "highestID:" << highestID 
                << ", nextMapID:" << nextMapID << dec << " Exits!" << endl;
         exit(1);
      }

   }
   if ( recreateMapIDsByName ) {
      // Create map name to ID and handled status list and
      // write it to disk.
      mc2log << info << "MM::createOverview" 
             << " OldMapping map ID to name by reading maps from disc" 
             << endl;
      mapIDsByName.initAndWriteFile( mapPath );
   }
   OldMapIdByName::mapIdAndHdlByName_t& mapIdAndHdlByName = 
      mapIDsByName.getWriteableMapIdByAndHdlName();

   // Get all maps in directory
   set<uint32> mapIDs; // OldMap IDs of maps in directory.
   MapGenUtil::getMapIDsInDir( mapIDs, mapPath );
   if ( mapIDs.size() == 0 ){
      mc2log << error << "MM::createOverview: No maps found in "
             << mapPath << " Perhaps they "
             << "are zipped? Exits!" << endl;
      exit(1);
   }

   // OldMap mapName by map ID.
   map<uint32, MC2String> mapNameById;
   OldMapIdByName::mapIdAndHdlByName_t::const_iterator nameIt =
      mapIdAndHdlByName.begin();
   while ( nameIt != mapIdAndHdlByName.end() ){
      MC2String mapName = nameIt->first;
      uint32 mapID = nameIt->second.first;
      mapNameById.insert( make_pair( mapID, mapName ) );
      ++nameIt;
   } 


   // Determine level of overview maps to generate, and overview map base
   // ID.
   if ( overviewByUnderview.size() == 0 ){
      mc2log << error << methodName 
             << "Size of overviewByUnderview from XML file is 0, exits!"
             << endl;
      exit(1);
   }
   // Find an underview to generate overview for.
   uint32 firstUnderviewId = MAX_UINT32;
   set<uint32>::iterator mapID_it = mapIDs.begin();
   while ( (mapID_it != mapIDs.end()) && 
           (firstUnderviewId == MAX_UINT32) )
   {
      map<uint32, MC2String>::const_iterator mapNameIt = 
         mapNameById.find(*mapID_it);
      if ( mapNameIt == mapNameById.end() ){
         mc2log << error << methodName 
                << "Could not find map name for ID:" << *mapID_it 
                << " Exits!" << endl;
         exit(1);
      }
      map<MC2String, MC2String>::const_iterator undIdIt = 
         overviewByUnderview.find(mapNameIt->second);      
      if ( undIdIt == overviewByUnderview.end() ){
         // This is not a map to use for these overviews, continue.
      }
      else {
         firstUnderviewId = *mapID_it;
         mc2dbg << methodName << "firstUnderviewId:" << firstUnderviewId 
                << endl;
      }
      ++mapID_it;
   }
   if ( firstUnderviewId == MAX_UINT32 ){
      mc2log << error << methodName
             << "Could not find first underview to use for overview." 
             << "Exits!" << endl;
      exit(1);
   }
   
   // Set overview base ID.
   uint32 overviewBaseID = 
      MAX( (firstUnderviewId & 0xf0000000) + (1 << 28),
           FIRST_OVERVIEWMAP_ID );
   mc2log << info << methodName << "Overview base ID:0x" << hex 
          << overviewBaseID << dec << endl;

   // Set level of overview maps.
   uint32 mapLevel = MapBits::getMapLevel( overviewBaseID );
   mc2log << info << methodName << "Level for overviews:" << mapLevel 
          << endl;


   // In case of creating super overviews, check that the file contains
   // overview maps.
   if ( mapLevel == 2 ){
      if ( !mapIDsByName.overviewsContained() ){
         mc2log << info << methodName 
                << "About to generate super overview but no overviews,"
                << "but overview maps missing in mapIDsByName. Exits!"
                << endl;
         exit(1);
      }
   }
   

   // Determine overview order.
   vector<MC2String> overviewOrder; // Process order of overview maps.
   mapID_it = mapIDs.begin();
   while (mapID_it != mapIDs.end()){
     
      map<uint32, MC2String>::const_iterator idAndNameIt =
         mapNameById.find(*mapID_it);
      if ( idAndNameIt == mapNameById.end() ){

         // Should this ID have been found? Depends on the level of
         // overview maps generated.
         bool missingID = false;
         if ( ( mapLevel == 1 ) && 
              ( MapBits::isUnderviewMap(*mapID_it) ) )
         {
            missingID = true;
         }
         else if ( ( mapLevel == 2 ) && 
                   ( MapBits::isOverviewMap(*mapID_it) ) )
         {
            missingID = true;
         }
         if ( missingID ){
            mc2log << error << "MM::createOverview"
                   << " OldMap ID needed missing in mapIdAndHdlByName. "
                   << " ID of map missing:\"" << *mapID_it << "\""
                   << " Exits!" << endl;
            exit(1);
         }
      }
      else {
         MC2String mapName = idAndNameIt->second;
         map<MC2String, MC2String>::const_iterator ovrIt = 
            overviewByUnderview.find(mapName);
         if ( ovrIt != overviewByUnderview.end() ){
            MC2String overviewName = ovrIt->second;

            if ( find( overviewOrder.begin(), overviewOrder.end(),
                       overviewName ) == overviewOrder.end() ) 
            { 
               // Not already added overview, add this one.
               overviewOrder.push_back(overviewName);
            }
         }
      }         
      ++mapID_it;
   }
   // Check sanity of overview order.
   if ( overviewOrder.size() == 0 ){
      mc2log << error << methodName
             << "( overviewOrder.size() == 0 )"
             << "Probably because no underview maps were found."
             << "Are they zipped? Exits!" << endl;
      exit(1);
   }
   // Print overview order.
   mc2log << info << methodName << "Printing overviews to gen. in order"
          << endl;
   for ( uint32 i=0; i<overviewOrder.size(); i++){
      mc2log << info << "   " << overviewOrder[i] << endl;
   }   

   // Make the overview map the key instead.
   multimap<MC2String, MC2String> underviewByOverview;
   mc2log << info << " Listing map hierarchy" << endl;
   for ( map<MC2String, MC2String>::const_iterator 
            theIt(overviewByUnderview.begin());        
         theIt != overviewByUnderview.end(); ++theIt ) 
   {
      MC2String underview = theIt->first;
      MC2String overview = theIt->second;
      underviewByOverview.insert(make_pair(overview, underview));
      mc2log << info << "   " << overview << " is overview map to "
             << underview << endl;
   }

   // Go through the overview maps and process their underview maps.
   mc2log << info << "" << endl;
   mc2log << info << "Creating overviews by adding their underviews." 
          << endl;
   mc2log << info << endl;

   for ( vector<MC2String>::iterator overviewIt = overviewOrder.begin();
               overviewIt != overviewOrder.end(); ++overviewIt )
   {

      // Find first underview map of this overview map.
      MC2String overviewName = *overviewIt;
      mc2log << info << "Processing overview map " << overviewName << endl;

      multimap<MC2String,MC2String>::const_iterator it;
      it = underviewByOverview.lower_bound(overviewName);

      uint32 firstUnderviewId = MAX_UINT32;
      bool firstUnderviewHdl = false;
      while ( it != underviewByOverview.upper_bound(overviewName) && 
              firstUnderviewId == MAX_UINT32 )
      {
         // Loop the maps until you find an underview map actually present
         // becaus maybe we are only generating a subset of the maps
         // normally included in this overview map.
         MC2String underviewName = it->second;
         OldMapIdByName::mapIdAndHdlByName_t::const_iterator undIdIt =
            mapIdAndHdlByName.find(underviewName);
         if ( undIdIt != mapIdAndHdlByName.end() ){
            firstUnderviewId = undIdIt->second.first;
            firstUnderviewHdl = undIdIt->second.second;
         }
         ++it;
      }
      if (firstUnderviewId == MAX_UINT32){
         // Could not find the first map of this overview.
         mc2log << warn << "Could not find the first map of the overview: "
                << overviewName << endl;
         continue;
      }

      // This code makes it possible to only generate one overview at a
      // time avoiding memory problems.
      mc2dbg8 << firstUnderviewHdl << endl;
      if ( oneMapAtATime && firstUnderviewHdl ){
         // This overview map has already been created, continue with next
         // one.
         mc2log << info << "First lower map 0x" << hex << firstUnderviewId
                << dec << " already processed, continue." << endl;
         continue;
      }


          

      // Create this overview map.


      mc2log << info << "First underview map id of overview map: " 
             << overviewName << " is " << firstUnderviewId << endl;
      

      // Figure out which is the new overviewmap base id.
      uint32 overviewBaseID = 
         MAX( (firstUnderviewId & 0xf0000000) + (1 << 28),
              FIRST_OVERVIEWMAP_ID );
      mapLevel = MapBits::getMapLevel( overviewBaseID );
      mc2dbg << "overviewBaseID = 0x" 
             << hex << overviewBaseID << dec 
             << ", map level = " << mapLevel << endl;

      OldGenericMap* firstUnderview =
         OldGenericMap::createMap(firstUnderviewId, mapPath);
      if (firstUnderview == NULL){
         // Could not create the first underview map of this overview.
         mc2log << error << "Could not create the first underview map "
                << "of the overview: " << overviewName << endl; 
         exit(1);
      }


      uint32 overviewMapID = MAX_UINT32;
      if ( mapLevel == 1 ){
         // Generating ordinary overview maps.
         overviewMapID = MapGenUtil::getNextOvrMapID( mapPath );
      }
      else if ( mapLevel == 2){
         // Generating super overview maps.
         overviewMapID = MapGenUtil::getNextSupOvrMapID( mapPath );
      }
      else {
         mc2log << error << methodName << "Strange mapLevel:" << mapLevel
                << " Exits!" << endl;
         exit(1);
      }
      mc2log << info << methodName << "ID of overview to generate:0x" 
             << hex << overviewMapID << dec << endl;

      OldOverviewMap* overview = new OldOverviewMap(overviewMapID);
      overview->createAllocators();
      MapGenUtil::setFileName(overview, mapPath);
      overview->setMapName( overviewName.c_str() );
      overview->setCountryCode( firstUnderview->getCountryCode() );
      if ( mapLevel < 2 ) {
         overview->setMapOrigin( firstUnderview->getMapOrigin() );
      }
      // Set driving side to the side of the underview map.
      overview->setDrivingSide( firstUnderview->driveOnRightSide() );
         
      delete firstUnderview;
      firstUnderview = NULL;


      // Order the underview maps of this overview map in ID order.
      map<uint32, MC2String>underviewById; // Underview name by ID.
      it = underviewByOverview.lower_bound(overviewName);
      while (it != underviewByOverview.upper_bound(overviewName) ){
         MC2String underviewName=it->second;
         OldMapIdByName::mapIdAndHdlByName_t::const_iterator mapIDIt =
            mapIdAndHdlByName.find(underviewName);
         if (mapIDIt != mapIdAndHdlByName.end()){
            uint32 underviewId = mapIDIt->second.first;
            underviewById.insert( pair<uint32, MC2String>
                                  (underviewId, underviewName));
         }
         it++;
      }

      // ---------------------------------------------
      // Go through underview maps of this overview map.
      map<uint32, MC2String>::iterator underviewIt = underviewById.begin();
      while (underviewIt != underviewById.end()){
         MC2String underviewName = underviewIt->second;
         mc2log << info << "Processing underview map " << underviewName
                << " of overview map " << overviewName << endl;

         uint32 curMapID = underviewIt->first;
         OldGenericMap* curMap = OldGenericMap::createMap(curMapID, mapPath);
         if ( curMap != NULL ) {

            bool usingIndexAreas = NationalProperties::useIndexAreas( 
                  curMap->getCountryCode(), curMap->getMapOrigin());
            
            if ( ( usingIndexAreas) && ( mapLevel < 2 ) ){
               mc2dbg8 <<"Creates temporary gfx data for BUAs in map:0x"
                       << hex << curMap->getMapID() << dec
                       << " because they are really nav index or locality index or"
                       << " index areas" << endl;
               curMap->createGfxDataFromSSIs( ItemTypes::builtUpAreaItem );
               // index areas have convex hull gfx datas, but we still
               // want to make them into bbox gfxdatas here
            }

            // Add the map to the (new) overview map.
            overview->addMap(curMap, false, mapLevel);
            OldMapIdByName::mapIdAndHdlByName_t::iterator undIt = 
               mapIdAndHdlByName.find(curMap->getMapName());
            if ( undIt != mapIdAndHdlByName.end() ){
               // Set this underview as handled.
               undIt->second.second = true; 
            }
            else {
               mc2log << error  << methodName
                      << "Could not update handled status for underview."
                      << " Exits!" << endl;
               exit(1);
            }
         }

         delete curMap;
         curMap=NULL;
         underviewIt++;
      } // for all underview maps of an overview map.

      
      // Loop over the underview maps once again and do stuff that 
      // demands that all maps have already been added to the overview map.
      mc2log << info << "Loads underviews once again. Adding ext conns."
             << endl;
      mc2log << info << endl;
      underviewIt = underviewById.begin();
      while (underviewIt != underviewById.end()){
         uint32 curMapID = underviewIt->first;

         OldGenericMap* curMap = OldGenericMap::createMap(curMapID, mapPath);
         if ( curMap != NULL ) {

            // Print the name of the map together with the map ID
            mc2dbg << "Adding ext conns: "
                   << curMapID << ". " << curMap->getMapName() << endl;

            // Add the external connection so of curMap to the overview
            overview->updateExternalConnectionsFromMap(underviewById,
                                                       curMap, 
                                                       mapLevel);
         }

         delete curMap;
         curMap=NULL;
         underviewIt++;
      }

      // Process the overview map before saving
      mc2log << info << "Processing overview before saving." << endl;
      mc2log << info << endl;

      
      // Create zip code agglomerations.
      mc2log << info << methodName << "Creating zip code agglomerations."
             << endl;
      if (overview != NULL) {
         overview->createZipCodeAgglomerations();
      }


      // Create group items in overview for grouping BUAs and municipals 
      // that are split between different underview maps.
      if (overview != NULL){
         
         // first do any index areas
         if ( NationalProperties::useIndexAreas( 
                  overview->getCountryCode(),
                  overview->getMapOrigin()) ) {
            // When using index areas:
            // same name areas located in same name areas are merged
            
            set<ItemTypes::itemType> itemTypes;
            itemTypes.insert(ItemTypes::builtUpAreaItem);
            uint32 result = 
               overview->groupSameNameAdminIndexAreas( itemTypes );

            if ( !result ){
               mc2log << error << "Group same name admin "
                      << "index areas failed." << endl;

               exit(1);
            }
         }
         // then do the normal buas and municipals
         uint32 mergeDist = 
            NationalProperties::getAdminAreasMergeDist(
               overview->getCountryCode() );
         mc2log << info << methodName 
                << "Creating group items for split BUAs." << endl;
         uint32 result = 
            overview->groupAdminAreas( ItemTypes::builtUpAreaItem,
                                       mergeDist );
         if ( !result ){
            mc2log << error  << methodName
                   << "Group admin areas for BUA failed. Exits!"
                   << endl;
            exit(1);
         }
         mc2log << info << methodName
                << "Creating group items for split municipals." 
                << endl;
         result = 
            overview->groupAdminAreas( ItemTypes::municipalItem,
                                       mergeDist );
         if ( !result ){
            mc2log << error  << methodName
                   << "Group admin areas for mun failed. Exits!" 
                   << endl;
            exit(1);
         }
      }

     
      
      mc2dbg1 << "Saving overview" << endl;
      overview->save();

      // Add a post to mapIdAndHdlByName file.
      pair<MC2String, pair<uint32, bool> > ovrPost;
      ovrPost.first = overview->getMapName();
      ovrPost.second.first = overview->getMapID();
      ovrPost.second.second = false; // handled status.
      pair<OldMapIdByName::mapIdAndHdlByName_t::iterator, bool> result =  
         mapIdAndHdlByName.insert(ovrPost);
      if ( ! result.second ){
         mc2dbg << methodName << "Could not insert overview to ID by name"
                << "file. Updates current post for " << result.first->first
                <<" with " << ovrPost.first
                << endl;
      
         OldMapIdByName::mapIdAndHdlByName_t::iterator insertIt = 
            result.first;
         if ( insertIt != mapIdAndHdlByName.end() ){
            insertIt->second = ovrPost.second;
         }
         else {
            mc2log << error << methodName
                   << "Someting wrong with map ID to map name file, exits!"
                   << endl;
         }
      }
      mc2log << info << methodName
             << "Writing map ID by name file" << endl;
      mapIDsByName.writeFile();

      delete overview;
      overview = NULL;


      if ( oneMapAtATime ){
         // We only wanted to create one map.
         mc2log << info << "One map handled, exits nicely" << endl;

         // Shut down the XML system
         XMLPlatformUtils::Terminate();         
      
         exit(0);
      }


   } // for all overview maps.

   if ( oneMapAtATime ){
      // If we get here, no map was generated. Tell the script by sending
      // an exitcode.
      uint32 exitCode = 77;
      mc2log << info << "Found no map to process. exits with exit code" 
             << exitCode << endl;

      // Shut down the XML system
      XMLPlatformUtils::Terminate();         
      
      exit(exitCode);
   }
} // END: createOverview


bool
changeMapID(uint32 oldID, uint32 newID)
{
   // The returnvalue for this metod
   bool retVal = false;
   
   // Check if the map with the newID alread exists
   const char* mapPath = "./";
   OldGenericMap* checkNewMap = 
      OldGenericMap::createMap( newID, mapPath );
   if (checkNewMap != NULL) {
      mc2log << error << here << " changeMapID  ERROR, map with id " 
             << newID
           << " already exsists" << endl;
   } else {
      OldGenericMap* changeMap = 
         OldGenericMap::createMap( oldID, mapPath );
      if (changeMap != NULL) {
         changeMap->setMapID(newID);
         changeMap->save();
         retVal = true;
         mc2log << info << here << "changeMapID  New ID set" << endl;
      } else {
         mc2log << error << here << "changeMapID  ERROR, map with id " 
                << oldID << " not found" << endl;
      }
      delete changeMap;
   }
   
   delete (checkNewMap);
   return (retVal);
}

void addMapSupCoverage(MapIndex& mapIndex,
                       char* mapSupCovXmlFile, 
                       char* mapSupNameXmlFile){
   XMLIndata::parseAndAddMapSupCoverageToIndex( mapIndex, mapSupCovXmlFile );
   XMLIndata::parseAndAddMapSupNamesToIndex( mapIndex, mapSupNameXmlFile );

} // addMapSupCoverage



void addAddrPointNames(char* addrPointFile){
   const char* mapPath = "./";

   AddrPointNames addrPointNames(addrPointFile);

   // Store which map each address point belongs to.
   for (uint32 mapID = CL_startAtMap; mapID <= CL_endAtMap; mapID++) {
      GMSMap* curMap = static_cast<GMSMap*>
         (GMSMap::createMap(mapID, mapPath));
      if (curMap == NULL) {
         break;
      }
      addrPointNames.storeSsiDistances(curMap);
      delete curMap;
   }

   // Add names from address points to map.
   for (uint32 mapID = CL_startAtMap; mapID <= CL_endAtMap; mapID++) {
      GMSMap* curMap = static_cast<GMSMap*>
         (GMSMap::createMap(mapID, mapPath));
      if (curMap == NULL) {
         break;
      }
      uint32 addedNames = addrPointNames.addAddrPointNamesToMap(curMap);
      if ( addedNames > 0 ){
         mc2dbg << "Saving map 0x" << hex << mapID << dec << endl;
         curMap->save();
      }
      else {
         mc2dbg << "Not saving map 0x" << hex << mapID << dec << endl;
      }
      delete curMap;
   }

} // addAddrPointNames


void finishIndexAreas(){
   const char* mapPath = "./";
   for (uint32 mapID = CL_startAtMap; mapID <= CL_endAtMap; mapID++) {
      GMSMap* curMap = static_cast<GMSMap*>
         (GMSMap::createMap(mapID, mapPath));
      if (curMap == NULL) {
         break;
      }
      mc2log << info << "finishIndexAreas for map " << curMap->getMapID()
             << endl;

      if ( ! NationalProperties::useIndexAreas(
               curMap->getCountryCode(), curMap->getMapOrigin()) ) {
         mc2dbg1 << "Not using index areas for country - exit" << endl;
         break;
      }

      bool finished = curMap->finishIndexAreas();

      if ( finished ) {
         mc2dbg << "Saving map 0x" << hex << mapID << dec << endl;
         curMap->save();
      }
      else {
         mc2dbg << "Not saving map 0x" << hex << mapID << dec << endl;
      }
      mc2log << info << "finishIndexAreas done for map " 
             << curMap->getMapID() << endl;
      delete curMap;
   }

} // finishIndexAreas

void removeDupItemsFromMidMif()
{
   mc2dbg << "removeDupItemsFromMidMif" << endl;
   uint32 totalRemove = 0;
   uint32 nbrMapsHadItemsRemoved = 0;
   
   uint32 startTime = TimeUtility::getCurrentTime();
   uint32 startTime1 = startTime;
   // Loop over all maps in current directory to
   // create and add items
   // Use the startAtMap and endAtMap if provided.
   mc2dbg1 << "Remove dupes added from mid mif, file: "
           << CL_removeDupItemsFromMidMif << endl;

   // Extract the itemtype from the filename
   ItemTypes::itemType itemType = 
         ItemTypes::getItemTypeFromString(CL_removeDupItemsFromMidMif);
   if ( (int) itemType > ( (int) ItemTypes::numberOfItemTypes -1) ) {
      mc2log << fatal << "ItemType could not "
             << "be extracted from the filename." << endl;
      MC2_ASSERT(false);
   } else {
      mc2dbg1 << "ItemType is " << (int) itemType << " "
              << StringTable::getString(
                  ItemTypes::getItemTypeSC(itemType),
                  StringTable::ENGLISH ) << endl;
   }

   MC2String fileName = CL_removeDupItemsFromMidMif;
   MC2String midName = (fileName + ".mid");
   MC2String mifName = (fileName + ".mif");
   ifstream test1(midName.c_str());
   ifstream test2(mifName.c_str());
   if (test1 && test2) {
      // Ok, item file exists
      test1.close();
      test2.close();

      // load the midId file into container
      set<uint32> midmifIDs;
      //ifstream midFile(fileName + ".mid");
      ifstream midFile( midName.c_str() );
      uint32 nbrItemsInFile = 0;
      const int maxLineLength =16380; //perhaps not enough for reading ssi line
      char inbuffer[maxLineLength];
      inbuffer[0] = '\0';
      uint32 midLineNbr = 1;
      while ( !midFile.eof() ) {
         midFile.getline(inbuffer, maxLineLength, ',');
         if (strlen(inbuffer) < 1 && !midFile.eof()){
            mc2log << warn << here << "Strange mid feature:"
                   << " nbrItemsInFile=" << nbrItemsInFile
                   << " midLineNbr=" << midLineNbr << endl;
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
                      << " midLineNbr=" << midLineNbr << endl;
               mc2log << warn << here << "Couldn't get id from inbuffer=\""
                      << inbuffer << "\"" << endl;
            }
            mc2dbg8 << "midId: " << midId << endl;
            if ( midId != MAX_UINT32 ) {
               midmifIDs.insert(midId);
            }
         }
         // read rest of line
         midFile.getline(inbuffer, maxLineLength);
         midLineNbr++;
      }
      mc2dbg1 << "Read mid file, nbrItemsInFile " << nbrItemsInFile 
              << "=" << midmifIDs.size() 
              << " took " 
              << (TimeUtility::getCurrentTime() - startTime1) / 1000.0
              << " s" << endl;
      startTime1 = TimeUtility::getCurrentTime();
   
      // Loop all maps, 
      // collect distance from each ibi to closest street segment
      typedef map<uint32, uint64> mapDistances_t; // mapID -> distance
      map<uint32, mapDistances_t> distToSSIinMap; // ibi midmifID -> mapID,distance
      const char* mapPath = "./";
      mc2dbg1 << "Loop maps to collect distance" << endl;
      uint32 totalFilled = 0;
      set<uint32> mapsWithFilling;
      for (uint32 mapID = CL_startAtMap; mapID <= CL_endAtMap; mapID++) {
         GMSMap* curMap = static_cast<GMSMap*>
                              (GMSMap::createMap(mapID, mapPath));
         if (curMap == NULL) {
            break;
         } else {
            GMSMidMifHandler mmh(curMap);
            uint32 nbrFilled = mmh.fillDistancesForItemsFromMidMif( 
                  itemType, midmifIDs, distToSSIinMap );
            totalFilled += nbrFilled;
            cout << "INFO map=" << curMap->getMapID() 
                   << " nbrFilled=" << nbrFilled 
                   << " totally=" << totalFilled << endl;
            if ( nbrFilled > 0 ) {
               mapsWithFilling.insert(mapID);
            }
         }
         delete curMap;
      }
      mc2log << info << "Filled info abt " << totalFilled
             << " items in " << mapsWithFilling.size() 
             << " maps, took " 
             << (TimeUtility::getCurrentTime() - startTime1) / 1000.0
             << " s" << endl;
      startTime1 = TimeUtility::getCurrentTime();
      
      // Loop all maps again,
      // remove IBI that have a smaller distance in other maps
      mc2dbg << "Loop maps to remove dupes" << endl;
      for (uint32 mapID = CL_startAtMap; mapID <= CL_endAtMap; mapID++) {
         GMSMap* curMap = static_cast<GMSMap*>
                              (GMSMap::createMap(mapID, mapPath));
         if (curMap == NULL) {
            break;
         } else {
            if ( mapsWithFilling.find(mapID) == mapsWithFilling.end() ) {
               cout << "INFO, no filling done for map "
                    << mapID << endl;
               continue;
            }
            GMSMidMifHandler mmh(curMap);
            uint32 nbrRemoved = mmh.removeDupItemsFromMidMif( 
                  itemType, midmifIDs, distToSSIinMap );
            totalRemove += nbrRemoved;
            cout << "INFO map=" << curMap->getMapID() 
                   << " nbrRemoved=" << nbrRemoved 
                   << " totally=" << totalRemove 
                   << " remains=" << totalFilled-nbrItemsInFile-totalRemove
                   << endl;
            if ( nbrRemoved > 0 ) {
               nbrMapsHadItemsRemoved++;
               curMap->save();
            }
         }
         delete curMap;

         // Todo: Improvemnt
         // If all extras now is removed, stop looping maps
         // Compare nbrItemsInFile with totalFilled and totalRemove
         if ( totalFilled == (nbrItemsInFile+totalRemove) ) {
            cout << "INFO map=" << curMap->getMapID()
                 << " all items removed - OK to stop looping" << endl;
            break;
         }
      }
   }
   uint32 totalTime = TimeUtility::getCurrentTime() - startTime;
   mc2log << info << "Removed " << totalRemove 
          << " items from " << nbrMapsHadItemsRemoved
          << " maps, took " 
          << (TimeUtility::getCurrentTime() - startTime1) / 1000.0
          << " s - totally "
          << (totalTime) / 1000.0 << " s " 
          << (totalTime) / 1000.0 / 60 << " m " << endl;
} // removeDupItemsFromMidMif

