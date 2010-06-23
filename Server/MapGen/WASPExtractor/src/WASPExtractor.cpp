/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "WASPExtractor.h"
#include "MySQLDriver.h"
#include "CommandlineOptionHandler.h"
#include "MapHashTable.h"
#include "OldStreetSegmentItem.h"
#include "GfxConstants.h"
#include <algorithm>
#include <set>
#include <fstream>
#include "GfxConstants.h"
#include "GfxUtility.h"
#include "GMSGfxData.h"
#include "CharEncSQLConn.h"
#include "UserEnums.h"
#include "StringUtility.h"
#include "STLStringUtility.h"
#include "OldExtraDataUtility.h"
#include "ItemCategories.h"
#include "TimeUtility.h"
#include "PropertyHelper.h"

// Defines, hardcoding the info key id of some POI info types
// as defined in POI db WASP
#define KEY_VIS_ADDRESS 0           // Street name (without house number), e.g. "Storgatan"



WASPExtractor::WASPExtractor(const char* database,
                             const char* host,
                             const char* username,
                             const char* password,
                             CharEncodingType::charEncodingType poiDBCharEnc,
                             uint32 maxDistMeter)
{
   mc2log << info << "POI database connection parameters:"  << endl;
   mc2log << info << "   Database: "  << database << endl;
   mc2log << info << "   Host: "      << host     << endl;
   mc2log << info << "   User name: " << username << endl;
   mc2log << info << "   Password: "  << password << endl;

   CharEncodingType::charEncodingType mc2CharEnc = 
      CharEncoding::getMC2CharEncoding();

   mc2log << info << "Character encodings:" << endl;
   mc2log << info << "   POI database: "
          << CharEncoding::encTypeToEncString (poiDBCharEnc) << endl;
   mc2log << info << "   Server code: " 
          << CharEncoding::encTypeToEncString(mc2CharEnc) << endl;


   bool dieOnError = true;
   m_sqlConnection =
      new CharEncSQLConn(new MySQLDriver(host, database, 
                                         username, password),
                         poiDBCharEnc,
                         mc2CharEnc,
                         dieOnError );
   m_subSqlConnection =
      new CharEncSQLConn(new MySQLDriver(host, database, 
                                         username, password),
                         poiDBCharEnc,
                         mc2CharEnc,
                         dieOnError);
   


   mc2dbg1 << "WASPExtractor: connections created" << endl;
   m_connected = (m_sqlConnection->connect()) &&
                 (m_subSqlConnection->connect());
   if (!m_connected) {
      mc2log << fatal << "WASPExtractor: Not connected! "
             << " - perhaps something wrong with POI database settings" << endl;
      MC2_ASSERT(false);
   }
   mc2dbg1 << "WASPExtractor: connected" << endl;

   m_maxDist_meters = maxDistMeter;
   m_maxSqDist_mc2 = uint64( maxDistMeter * maxDistMeter * 
                     GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE);
}

WASPExtractor::~WASPExtractor()
{  
   delete m_sqlConnection;
   delete m_subSqlConnection;
}

void
WASPExtractor::closeThenOpenDatabaseConnection(const char* database, 
					       const char* host, 
					       const char* username, 
					       const char* password,
                      CharEncodingType::charEncodingType poiDBCharEnc)
{
   mc2log << info << "Closing POI database connection"  << endl;

  delete m_sqlConnection;
  delete m_subSqlConnection;

  mc2log << info << "Reopening POI database connection"  << endl;


  CharEncodingType::charEncodingType mc2CharEnc = 
    CharEncoding::getMC2CharEncoding();

  bool dieOnError = true;
  m_sqlConnection =
    new CharEncSQLConn(new MySQLDriver(host, database, 
				       username, password),
		       poiDBCharEnc,
		       mc2CharEnc,
		       dieOnError );
  m_subSqlConnection =
    new CharEncSQLConn(new MySQLDriver(host, database, 
				       username, password),
		       poiDBCharEnc,
		       mc2CharEnc,
		       dieOnError);
   

  
  mc2dbg1 << "WASPExtractor: connections created" << endl;
  m_connected = (m_sqlConnection->connect()) &&
     (m_subSqlConnection->connect());
  mc2dbg1 << "WASPExtractor: connected" << endl;
}

bool
WASPExtractor::doQuery(SQLQuery* sqlQuery, const char* query,
                       const char* whereTag)
{
   mc2dbg8 << "WASPExtractor::doQuery(), query: " << query
           << ", tag: " << whereTag << endl;
   if (query != NULL) {
      if (! sqlQuery->prepare(query) ) {
         mc2log  << error << "Problem preparing query at " << whereTag << ": "
                 << sqlQuery->getErrorString() << endl;
         mc2dbg4 << "Failed query: " << query << endl;
         return false;
      }
   }

   if (! sqlQuery->execute() && sqlQuery->getError() > 0) {
      mc2log << error << "Problem executing query at " << whereTag << ": "
             << sqlQuery->getErrorString() << endl;
      mc2log << error << "Failed query: " << query << endl;
      return false;
   }

   return true;
}

bool
WASPExtractor::checkValidMapVersion(const char* mapVersion, bool addToMap, 
                                    uint32& mapVersionID)
{
   bool mapVersionValid = false;

   const char* task = "";
   if (addToMap)
      task = "WASPExtractor::addToMap";
   else
      task = "WASPExtractor::updateInMap";
   
   //Get the ID for the mapVersion
   char versionIDQuery[2000];
   sprintf(versionIDQuery,
           "SELECT ID FROM EDVersion "
           "WHERE version LIKE %s%s%s;", "\"", mapVersion, "\"");
   SQLQuery* sqlQuery = m_sqlConnection->newQuery(); 
   if ( ! doQuery(sqlQuery, versionIDQuery, task) ) {
      delete sqlQuery;
      return -1;
   }

   mapVersionID = 0;
   uint32 nbrRows = 0;
   while (sqlQuery->nextRow()) {
      nbrRows++;
      mapVersionID = atoi(sqlQuery->getColumn(0));
   }
   //Check that no errors occured in sqlQuery
   if ( sqlQuery->getError() != 0 ) {
      mc2log << error << here << " Problem with sql query, error code="
             << sqlQuery->getError() << ", errror string='"
             << sqlQuery->getErrorString() << "'" << endl;
      MC2_ASSERT(false);
   }

   //Check if given version is valid
   if ( nbrRows == 0 ){
      // Got no result from mapVersion query.
      mapVersionValid = false;
   }
   else if (mapVersionID != 0){
      // version id valid
      mapVersionValid = true;
   }
   delete sqlQuery;

   return mapVersionValid;
}


bool
WASPExtractor::getWaspDateFromMap(OldGenericMap* theMap, char* waspDate)
{
   if (theMap == NULL)
      return false;

   uint32 waspTime = theMap->getWaspTime();
   if (waspTime == MAX_UINT32)
      return false;

   time_t time = time_t(waspTime);
   bool retVal = sprintf(waspDate,
                         "%04d-%02d-%02d %02d:%02d:%02d",
                         localtime(&time)->tm_year + 1900,
                         localtime(&time)->tm_mon + 1,
                         localtime(&time)->tm_mday,
                         localtime(&time)->tm_hour,
                         localtime(&time)->tm_min,
                         localtime(&time)->tm_sec);
   return retVal;
}

int
WASPExtractor::addToMap(OldGenericMap* theMap,
                        multimap<uint32, poiAddData_t>& notAddedIDs,
                        uint32 mapVersionID,
                        bool skipControlledAccessPOIs)
{
 
   vector<uint32> addedToThisMap;

   // Used for collecting POI rights set before saving the map.
   UserRightsItemTable::itemMap_t userRights;

 

   if (!m_connected) {
      mc2log << fatal << "Not connected" << endl;
      return -1;
   }


   // Remove all POIs in the mcm maps before adding POIs from WASP
   // (all POIs we are interested in, exist in and are added from WASP)
   bool removeExcistingPOIs = true;
   if (removeExcistingPOIs) {
      mc2dbg1 << "Remove any existing POI:s" << endl;
      uint32 r = removeAllPOI(theMap);
      mc2log << info << "Removed " << r << " POIs from map " 
             << theMap->getMapID() << " before adding the new ones" << endl;
   } else {
      mc2log << info << "Did not try to remove POIs before adding new ones" 
             << endl;
   }
   
   MC2BoundingBox bbox;
   theMap->getMapBoundingBox(bbox);
  
   mc2log << "mapVersionID=" << mapVersionID << " (from POI.EDVersion)"
          << endl;
 

   char subControlledAccessWhereQuery[4096];
   if ( skipControlledAccessPOIs ) {
      sprintf(subControlledAccessWhereQuery, " AND rights = \"FFFFFFFFFFFFFFFF\" ");
   } else {
      sprintf(subControlledAccessWhereQuery, " ");
   }


   char whereQuery[4096];
   sprintf(whereQuery, "FROM POIMain WHERE "
                       "country=%i AND "
                       "inUse=1 AND deleted=0 AND lat<%i AND lat>%i "
                       "AND (lon-%i)<0 and (lon-%i)>0 "
                       "%s "
                       "AND (validFromVersion <= %i "
                       "OR validFromVersion IS NULL) "
                       "AND (validToVersion >= %i "
                       "OR validToVersion IS NULL);",
                       int(theMap->getCountryCode()), 
                       bbox.getMaxLat(), bbox.getMinLat(),
                       bbox.getMaxLon(), bbox.getMinLon(),
                       subControlledAccessWhereQuery,
                       mapVersionID, mapVersionID);


   // Get the POI's inside bounding box of the map
   char query[4096];
   sprintf(query, "SELECT ID,lat,lon,rights %s", whereQuery);
   mc2log << "select query: " << query << endl;
   SQLQuery* sqlQuery = m_sqlConnection->newQuery();
   if ( ! doQuery(sqlQuery, query, "WASPExtractor::addToMap") ) {
      delete sqlQuery;
      return -1;
   }

   SQLQuery* subSqlQuery = m_subSqlConnection->newQuery();
   int nbrAdded = 0;
   while (sqlQuery->nextRow()) {
      uint32 poiWaspID = uint32(atoi(sqlQuery->getColumn(0)));
      set<uint32>::iterator it;
      it = m_addedIDs.find(poiWaspID);
      if (it == m_addedIDs.end()) {
         
         // Get entry-points to make sure we use the correct coordinate.
         // Current criterias
         // # EP |  Find SSI  | Store in GfxData
         // -----+------------+------------------
         //   0  | Symbol     | Symbol
         //   1  | EP         | Symbol
         //   >1 | EP(0)      | Symbol

         int32 symbolLat = atoi(sqlQuery->getColumn(1));
         int32 symbolLon = atoi(sqlQuery->getColumn(2));
         MC2String right(sqlQuery->getColumn(3));
         mc2dbg8 << "Raw right: " << right << endl;
         MapRights poiRights = MapRights( right );

         OldPointOfInterestItem* poi = new OldPointOfInterestItem(MAX_UINT32);
         poiAddData_t notInsertedData;
         set<uint16> poiCategories;
         poi = createPOI( theMap, subSqlQuery, poi,
                          poiWaspID, symbolLat, symbolLon,
                          true, notInsertedData, 
                          poiCategories );

         if (poi != NULL) {

            // Check if the created POI should be added to the map or not
            // Not necessary if all POI:s in the map have been removed...
            if (removeExcistingPOIs || !alreadyAdded(poi, theMap)) {
               uint32 id = theMap->addItem(poi, 14);
               if (id == MAX_UINT32) {
                  delete sqlQuery;
                  delete subSqlQuery;
                  delete poi;
                  mc2log << error << "Could not add fitting  POI to map." 
                         << endl;
                  MC2_ASSERT(false);
               }

               mc2dbg << "Added poi " << theMap->getFirstItemName(poi) 
                      << " at " 
                      << theMap->getFirstItemName(
                           poi->getStreetSegmentItemID()) 
                      << ", map " << theMap->getMapID() << "(sqlID "
                      << poiWaspID << ")" << endl;
               ++nbrAdded;
               addedToThisMap.push_back(poiWaspID);
               // Store the rights value for coming addition.
               userRights.insert(make_pair(poi->getID(), poiRights));
               mc2dbg4 << "Stored right for poi: " << poi->getID() 
                       << ", mapright: " << poiRights << endl;
               // Add the categories.
               poi->addCategories(*theMap, poiCategories);

            } else {
               mc2dbg4 << "poi already added (sql-ID " << poiWaspID << ")" 
                       << endl;
               delete poi;
            }
         } else {
            mc2dbg4 << "Did not add poi, to large distance?! (sql-ID " 
                    << poiWaspID << "), dist=" << notInsertedData.dist_meters
                    << ", " << theMap->getMapID() 
                    << " might be wrong map..." << ", closest SSI at ("
                    << notInsertedData.lat << "," << notInsertedData.lon << ")"
                    << endl;
            if (notInsertedData.dist_meters > 0) {
               notAddedIDs.insert( pair<uint32, poiAddData_t>
                                   (poiWaspID, notInsertedData) );
            }
         }
      }
   }
   //Check that no errors occured in sqlQuery
   if ( sqlQuery->getError() != 0 ) {
      mc2log << error << here << " Problem with sql query, error code="
             << sqlQuery->getError() << ", errror string='"
             << sqlQuery->getErrorString() << "'" << endl;
      MC2_ASSERT(false);
   }
      
   delete sqlQuery;
   delete subSqlQuery;

   m_addedIDs.insert(addedToThisMap.begin(), addedToThisMap.end());
   theMap->setUserRights(userRights);

   return nbrAdded;
} // addToMap


OldPointOfInterestItem*
WASPExtractor::findSsiAndOffset(OldGenericMap* theMap,
                                  OldPointOfInterestItem* poi,
                                  int32 ssiLat,
                                  int32 ssiLon,
                                  bool newPOI,
                                  poiAddData_t& notAddedData)
{
   
   // Check that the coordinate is inside this map.
   if (theMap->getGfxData()->insidePolygon(ssiLat, ssiLon) == 0) {
      mc2dbg4 << "Not inside map-polygon (" << ssiLat << "," 
              << ssiLon << ")" << endl;
      if ( newPOI ) {
         return NULL;
      }
   }

   // Grab the hash table
   OldMapHashTable* mht = theMap->getHashTable();
   MC2_ASSERT(mht != NULL);
   mht->clearAllowedItemTypes();
   mht->addAllowedItemType(ItemTypes::streetSegmentItem);

   // Find near street segment
   uint64 sqDist_mc2;
   uint32 nearestSSI = mht->getClosest(ssiLon, ssiLat, sqDist_mc2);
   OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*>
                                 (theMap->itemLookup(nearestSSI));
   if ( (ssi == NULL) || (sqDist_mc2 > m_maxSqDist_mc2)) {
      float64 dist_meters = sqrt(sqDist_mc2) * 
                     GfxConstants::MC2SCALE_TO_METER;
      // no ssi close to the coordinate, return NULL
      if (ssi == NULL) {
         mc2log << fatal << " No SSI close to POI (" << ssiLat << "," 
                << ssiLon << ") dist=" << dist_meters << "m" << endl;
         return NULL;
      }
      // Adding new POI, distance too large, don't add
      if (newPOI) {
         mc2dbg4 << "POI with large distance to SSI (POI not added), " 
                << dist_meters << "m (" << ssiLat << "," << ssiLon 
                << ")" << endl;

         // Calculate closest coord etc.
         int32 tmpOffset = theMap->getItemOffset(ssi, ssiLat, ssiLon);
         uint16 offset = MAX_UINT16 / 2;
         if (tmpOffset >= 0) {
            offset = uint16(tmpOffset);
         }
         int32 latOnSSI, lonOnSSI;
         theMap->getItemCoordinates(ssi->getID(), offset, latOnSSI, lonOnSSI);
         notAddedData.lat = latOnSSI;
         notAddedData.lon = lonOnSSI;
         notAddedData.dist_meters = uint32(dist_meters);
         notAddedData.mapID = theMap->getMapID();
         notAddedData.streetName = theMap->getFirstItemName(ssi);
         return NULL;
      }
      // Modified POI in dynamic WASPing, distance too large
      // But please keep the POI in the map
      mc2dbg << "Modified POI " << poi->getWASPID() << " dist=" << dist_meters 
             << " meters" << endl;
   }
  
   // No need to copy the groups from the ssi to the poi. 
   // The ssi groups should be used instead.
   poi->setStreetSegmentItemID(ssi->getID());

   // Get/set offset
   int32 offset = theMap->getItemOffset(ssi, ssiLat, ssiLon);
   if (offset >= 0) {
      poi->setOffsetOnStreet(uint16(offset));
      mc2dbg4 << poi->getID() << " offset=" <<  offset << ", ssi=" 
              << theMap->getFirstItemName(ssi) << "=" << ssi->getID() << endl;
   } else {
      mc2log << warn << "Failed to set offset for poi " << poi->getID() << endl;
   }

   return poi;
}

bool 
WASPExtractor::alreadyAdded(const OldPointOfInterestItem* poi, 
                            const OldGenericMap* theMap)
{
   // Naive check to find out if we already have added this poi
   // to the map
   const uint32 z = 14;
   for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); ++i) {
      OldPointOfInterestItem* checkPOI = 
         dynamic_cast<OldPointOfInterestItem*>(theMap->getItem(z, i));
      if ( (checkPOI != NULL) && 
           (checkPOI->getStreetSegmentItemID() == poi->getStreetSegmentItemID()) &&
           (checkPOI->getOffsetOnStreet() == poi->getOffsetOnStreet()) &&
           (checkPOI->hasSameNames(poi))) {
         return true;
      }
   }
   return false;
}

uint32
WASPExtractor::removeAllPOI(OldGenericMap* theMap)
{
   uint32 nbrRemoved = 0;

   const uint32 z = 14;
   for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); ++i) {
      OldPointOfInterestItem* poi = 
         dynamic_cast<OldPointOfInterestItem*>(theMap->getItem(z, i));
      if (poi != NULL) {
         theMap->removeItem(poi->getID(), false);
         ++nbrRemoved;
      }
   }
   return nbrRemoved;
}

OldPointOfInterestItem*
WASPExtractor::findPOIWithWASPID(OldGenericMap* theMap, uint32 WASPID)
{
   const uint32 z = 14;
   OldPointOfInterestItem* poi = NULL;
   
   uint32 i = 0;
   bool cont = true;
   while (cont && (i<theMap->getNbrItemsWithZoom(z)) ) {
      OldPointOfInterestItem* tempPoi =
         dynamic_cast<OldPointOfInterestItem*>(theMap->getItem(z, i));
      if (tempPoi != NULL) {
         uint32 tempWASPID = tempPoi->getWASPID();
         if (tempWASPID == WASPID) {
            poi  = tempPoi;
            cont = false;
         }         
      }
      i++;
   }   
  return poi; 
}

bool
WASPExtractor::getEntryPointForWASPID(
                  SQLQuery* subSqlQuery,
                  uint32 waspId, int32 symbolLat, int32 symbolLon,
                  int32& ssiLat, int32& ssiLon)
{
   bool entryPointFound = true;

   if (!m_connected) {
      mc2log << fatal << "Not connected" << endl;
      return false;
   }
   
   //Get entry point
   char epQuery[4096];
   sprintf(epQuery,
         "SELECT lat, lon FROM POIEntryPoints WHERE poiID=%i order by ID;",
         waspId);
   if ( ! doQuery(subSqlQuery, epQuery, "WASP::getEPForWASPID") ) {
      return false;
   }
   vector< pair<int32, int32> > entryPoints;
   while (subSqlQuery->nextRow()) {
      entryPoints.push_back(
            make_pair( atoi(subSqlQuery->getColumn(0)),
                       atoi(subSqlQuery->getColumn(1))) );
   }
   // Initiate ssiCoord to symbol coord.
   // If there is no EP for this POI, we go with the symbol coord
   ssiLat = symbolLat;
   ssiLon = symbolLon;
   if (!entryPoints.empty()) {
      ssiLat = entryPoints.begin()->first;
      ssiLon = entryPoints.begin()->second;
      mc2dbg4 << "Will use entryPoint at (" << ssiLat << "," << ssiLon
              << ") to find SSI for POI with SQL-ID="
              << waspId << endl;
   } else {
      mc2dbg4 << "Will use symbol coord at (" << ssiLat << "," << ssiLon
              << ") to find SSI for POI with SQL-ID="
              << waspId << endl;
   }
   
   return entryPointFound;
}

OldPointOfInterestItem*
WASPExtractor::createPOI(OldGenericMap* theMap, 
                         SQLQuery* subSqlQuery, 
                         OldPointOfInterestItem* poi,
                         uint32 poiWaspID, 
                         int32 symbolLat, 
                         int32 symbolLon,
                         bool newPOI,
                         poiAddData_t& notAddedData,
                         set<uint16>& poiCategories)
{
   if (!m_connected) {
      mc2log << fatal << "Not connected" << endl;
      delete subSqlQuery;
      MC2_ASSERT(false);
   }

   //Get entry point
   int32 ssiLat;
   int32 ssiLon;
   if (! getEntryPointForWASPID( subSqlQuery,
            poiWaspID, symbolLat, symbolLon, ssiLat, ssiLon) ) {
      // not connected, or failed to do the query
      mc2log << fatal << "Entry point could not be set for waspId"
             << poiWaspID << endl;
      delete subSqlQuery;
      MC2_ASSERT(false);
   }

   poi->setWASPID(poiWaspID);
   
   // Find ssi and offset
   notAddedData.dist_meters = 0;
   poi = findSsiAndOffset(theMap, poi, ssiLat, ssiLon, newPOI, notAddedData);

   if (poi != NULL) {
      // Set default values for source and side
      poi->setSource(SearchTypes::UNKNOWN);
      poi->setSide(SearchTypes::unknown_side);

      // Add gfxData at (symbolLat, symbolLon) if not to close to 
      // geocoded position
      const uint32 sqMinDistMeters = m_maxDist_meters * m_maxDist_meters;
      if ( GfxUtility::squareP2Pdistance_linear(
               symbolLat, symbolLon, ssiLat, ssiLon) > sqMinDistMeters) {
         GfxDataFull* newGfx = GMSGfxData::createNewGfxData(
                                      theMap, &symbolLat, &symbolLon, 1, false);
         poi->setGfxData(newGfx);
      } else {
         mc2dbg4 << "Not necessary to create GfxData, distance only "
                 << sqrt(GfxUtility::squareP2Pdistance_linear(
                     symbolLat, symbolLon, ssiLat, ssiLon))
                 << "m" << endl;
         // If running dynamic WASPing ending up here, the poi should have
         // no gfxData. If it had one before remove it! (memory leak?)
         if ( poi->getGfxData() != NULL ) {
            poi->setGfxData( NULL );
         }
      }


      // Add poi-type. Currently only one type
      char typeQuery[4096];
      sprintf(typeQuery,
            "SELECT typeID FROM POITypes WHERE poiID=%i order by typeID;",
            poiWaspID);
      if ( ! doQuery(subSqlQuery, typeQuery, "WASP::createPOI (type)") ) {
         delete subSqlQuery;
         MC2_ASSERT(false);
      }
      poi->setPointOfInteresttype(ItemTypes::invalidPOIType);
      if (subSqlQuery->nextRow()) {
         poi->setPointOfInteresttype(ItemTypes::pointOfInterest_t(
                  atoi(subSqlQuery->getColumn(0))));
         mc2dbg4 << "Added type to poi: " << subSqlQuery->getColumn(0)
                 << endl;
      } else {
         // no type in the POITypes table. Error, but don't exit.
         mc2log << error << "Poi " <<  poiWaspID 
                << " has no type in WASP" << endl;
      }
      
      // Add poi categories
      char catQuery[4096];
      sprintf(catQuery,
              "SELECT catID FROM POICategories WHERE poiID=%i;",
              poiWaspID);
      mc2dbg8 << "Category query: " << catQuery << endl;
      if ( ! doQuery(subSqlQuery, catQuery, "WASP::createPOI (category)") ) {
         delete subSqlQuery;
         MC2_ASSERT(false);
      }
      while (subSqlQuery->nextRow()) {
         uint16 catID = atoi(subSqlQuery->getColumn(0));
         if ( catID != ItemCategories::invalidCategoryID ){
            poiCategories.insert(catID);
            mc2dbg8 << "Scheduled category: "<< catID 
                    << " for addition to POI: " << poiWaspID
                    << endl;
         }
         else {
            // no category for this POI.
            mc2dbg << "No category for POI: " << poiWaspID
                   << endl;
         }
      }
      
      

      // Add name to the poi
      // Do not add any synonym names from the POI db (nameType 7)
      poi->removeAllNames();
      char nameQuery[4096];
      sprintf(nameQuery, "SELECT name, lang, type FROM POINames "
                        "WHERE poiID=%i and type != 7;", poiWaspID);
      if ( ! doQuery(subSqlQuery, nameQuery, "WASP::createPOI (name)") ) {
         delete subSqlQuery;
         MC2_ASSERT(false);
      }
      // Collect the names to a vector for sorting
      vector<poiName_t> poiNames;
      while (subSqlQuery->nextRow()) {
         poiName_t poiName;
         poiName.name = 
            StringUtility::newStrDup(subSqlQuery->getColumn(0));
         poiName.poiNameLang = 
            LangTypes::language_t(atoi(subSqlQuery->getColumn(1)));
         poiName.poiNameType = 
            ItemTypes::name_t(atoi(subSqlQuery->getColumn(2)));
         poiNames.push_back(poiName);
      }
      // Sort the poiNames: 1 on, 2 an, 3 ...
      sort(poiNames.begin(), poiNames.end(), LessPoiNameOrder());
      // Add the names sorted, 1: on, 2: an, 3: ...
      for ( vector<poiName_t>::iterator it = poiNames.begin();
            it != poiNames.end(); it++ ) {
         if ( ! cityCentreWithLocationAltName(poi, *it, theMap) ) {
            uint32 strIndex = 
                     theMap->addNameToItem( poi, (*it).name, 
                                            (*it).poiNameLang,
                                            (*it).poiNameType );
            if (strIndex == MAX_UINT32) {
               mc2log << error << "createPOI: Failed to add name to POI" 
                      << endl;
               delete subSqlQuery;
               MC2_ASSERT(false);
            }
            mc2dbg4 << "Added name to poi waspid " << poiWaspID << ": "
                    << (*it).name << endl;
         }
      }


   }
   return poi;
}

int
WASPExtractor::createEntryPoints(OldGenericMap* theMap, 
      map<uint32, WASPExtractor::epData_t>& epData,
      const char* sourceString )
{
   MC2BoundingBox bbox;
   theMap->getMapBoundingBox(bbox);
   char query[4096];

   mc2dbg << "Create entry points in map " << theMap->getMapID()
          << " with countryCode " << int(theMap->getCountryCode())
          << ", name=" << theMap->getMapName() << endl;
   mc2dbg << "createEP in country=" << int(theMap->getCountryCode()) 
          << " origin=" << theMap->getMapOrigin()
          << " fileName=" << theMap->getFilename() << endl;
   // If sourceString given, only extract POIs from that source.
   char srcSubCondition[4096];
   sprintf(srcSubCondition, "%s", "");
   if ( sourceString != NULL ) {
      mc2dbg << "Create eps only for POIs with source " << sourceString << endl;
      // find source id
      char srcQuery[4096];
      sprintf( srcQuery,
               "SELECT id from POISources where source like '%s';",
               sourceString );
      SQLQuery* sqlQuery = m_sqlConnection->newQuery();
      if ( ! doQuery(sqlQuery, srcQuery, "WASPExtractor::createEP srcQuery")) {
         delete sqlQuery;
         return -1;
      }
      if ( sqlQuery->nextRow() ) {
         uint32 id = uint32( atoi(sqlQuery->getColumn(0)) );
         sprintf(srcSubCondition, " AND source=%d", id);
      } else {
         mc2log << fatal << "Did not find sourceID for source '"
                << sourceString << "'" << endl;
         delete sqlQuery;
         return -1;
      }
      delete sqlQuery;
   }


   // Select poi wasp id and coordinates for POIs within this map
   // that don't have entry points.
	// Don't care if inUse
   sprintf(query, "SELECT POIMain.ID, POIMain.lat, POIMain.lon, POIMain.source "
                  "FROM POIMain LEFT JOIN POIEntryPoints ON POIMain.ID = POIEntryPoints.poiID WHERE country=%i "
                  "AND deleted=0 "
                  "AND POIEntryPoints.ID IS NULL "
                  "AND POIMain.lat<%i AND POIMain.lat>%i "
                  "AND (POIMain.lon-%i)<0 and (POIMain.lon-%i)>0%s;",
                  int(theMap->getCountryCode()),
                  bbox.getMaxLat(), bbox.getMinLat(),
                  bbox.getMaxLon(), bbox.getMinLon(),
                  srcSubCondition);
   cout << "query: " << query << endl;

   SQLQuery* sqlQuery = m_sqlConnection->newQuery();
   if ( ! doQuery(sqlQuery, query, "WASPExtractor::createEntryPoints") ) {
      delete sqlQuery;
      return -1;
   }
   mc2dbg << "query done, start looping selected pois" << endl;

   // Loop all selected POIs and check if they have entrey points already.
   // If not -> create entry point.
   char subQuery[4096];
   SQLQuery* subSqlQuery = m_subSqlConnection->newQuery();
   OldMapHashTable* hashTable = theMap->getHashTable();
   hashTable->clearAllowedItemTypes();
   hashTable->addAllowedItemType(ItemTypes::streetSegmentItem);
   while (sqlQuery->nextRow()) {
      uint32 poiSQLID = uint32(atoi(sqlQuery->getColumn(0)));
      int32 lat = atoi(sqlQuery->getColumn(1));
      int32 lon = atoi(sqlQuery->getColumn(2));
      // Check if the current POI already has entry points
      sprintf(subQuery, "SELECT COUNT(*) FROM POIEntryPoints WHERE poiID=%i;", 
              poiSQLID);

      if ( ! doQuery(subSqlQuery, subQuery, "WASP::createEP (#EP)") ) {
         delete sqlQuery;
         delete subSqlQuery;
         return -1;
      }

      if ((subSqlQuery->nextRow()) && (atoi(subSqlQuery->getColumn(0)) == 0)) {
         // No entrypoints 
         
         // Find the ssi that will be used for the entry point
         // 1. - deprecated
         // 2. closest to symbol coord, same name as poi info vis.address
         //    distance tolerance is 200 meters
         // 3. closest to symbol coord, regardless of name
         uint32 closestID = MAX_UINT32;
         uint64 closestSqDist_mc2 = 0;
         

         // Case 2
         // 2. closest to symbol coord, same name as poi info vis.address
         //    distance tolerance is 200 meters
         if ( closestID == MAX_UINT32 ) {
            // Try to get the address from POIInfo
            sprintf(subQuery, "SELECT val FROM POIInfo WHERE poiID=%i AND "
                              "keyID=%i;", 
                    poiSQLID, KEY_VIS_ADDRESS);
            if ( ! doQuery(subSqlQuery, subQuery, "WASP::createEP (val)") ) {
               delete sqlQuery;
               delete subSqlQuery;
               return -1;
            }

            if (subSqlQuery->nextRow()) {
               // POI had visiting address in POIInfo table
               const char* streetName = subSqlQuery->getColumn(0);
               mc2dbg4 << "Will try to find \"" << streetName
                       << "\" close to (" << lat << "," << lon
                       << ") for POI " << poiSQLID << endl;

               // Get all SSI that are "close"
               const uint32 radius_meter = 200;
               bool shouldKill = false;
               Vector* closeSSIs = hashTable->getAllWithinRadius_meter(
                     lon, lat, radius_meter, shouldKill);
               if ((closeSSIs != NULL) && (closeSSIs->getSize() > 0)) {
                  // Collect the ones with correct name
                  set<uint32> sameNames;
                  for (uint32 i=0; i<closeSSIs->getSize(); ++i) {
                     OldItem* item = 
                           theMap->itemLookup(closeSSIs->getElementAt(i));
                     for (uint32 n=0; n<item->getNbrNames(); ++n) {
                        const char* itemName = 
                           theMap->getName(item->getStringIndex(n));
                        mc2dbg4 << "Checking names: " << itemName << "(" 
                                << item->getID() << ") =?= "
                                << streetName << endl;
                        if (StringUtility::strcasecmp(
                                    itemName, streetName) == 0) {
                           mc2dbg4 << "   Found!" << endl;
                           sameNames.insert(item->getID());
                        }
                     }
                  }

                  // Find the closest
                  closestSqDist_mc2 = MAX_UINT32;
                  for (set<uint32>::const_iterator it=sameNames.begin(); 
                        it!=sameNames.end(); ++it) {
                     uint64 curDist = theMap->itemLookup(*it)->getGfxData()
                        ->squareDistToLine_mc2(lat, lon);
                     if (curDist < closestSqDist_mc2) {
                        closestSqDist_mc2 = curDist;
                        closestID = *it;
                     }
                  }
                  
               } else {
                  
                  // No street within 200 m -- get the 
                  // closest street! (case 3)
               }
               if (shouldKill)
                  delete closeSSIs;
               if ( closestID != MAX_UINT32 ) {
                  mc2dbg8 << "Found ssi " << closestID << " \"" 
                          << streetName << "\" close to (" << lat << "," 
                          << lon << ") for POI " << poiSQLID 
                          << " dist=" << (sqrt(closestSqDist_mc2)*
                                          GfxConstants::MC2SCALE_TO_METER)
                          << " m" << endl;
               }
               
            }
            // else No address, get closest street (case 3)
         }
         
         // Case 3
         // 3. closest to symbol coord, regardless of name
         if ( closestID == MAX_UINT32 ) {
            // No address, or no ssi within 200 m, or no correctName ssi
            // within 200 m
            // Simply get the closest ssi
            closestID = hashTable->getClosest(lon, lat, closestSqDist_mc2);
            mc2dbg8 << "Found closest ssi " << closestID << " to ("
                    << lat << "," << lon << ") for POI " << poiSQLID
                    << " dist=" << (sqrt(closestSqDist_mc2)*
                           GfxConstants::MC2SCALE_TO_METER)
                    << " m" << endl;
         }

         // Don't create entry points if the distance is too large
         // e.g in countries where we don't have full coverage
         if (closestID != MAX_UINT32) {
            float64 meters =
               sqrt(closestSqDist_mc2)*GfxConstants::MC2SCALE_TO_METER;
            if ( meters > 10000 ) { // 10 km
               // dont use the entry point
               mc2dbg8 << "Not using closestID " << theMap->getMapID()
                       << ":" << closestID << " for " << poiSQLID
                       << " dist = " << meters << " meters" << endl;
               closestID = MAX_UINT32;
            }
         }

         // If we now have a closestID, we can create the entry point
         // and store in epData
         if (closestID != MAX_UINT32) {

            // Get entry point, closest offset on the found ssi using
            // the POI symbol coordinate
            int32 entryLat, entryLon;
            uint16 offsetOnSSI = theMap->itemLookup(closestID)
               ->getGfxData()->getOffset(lat, lon, entryLat, entryLon);

            // Don't choose entry point coord in the node of the ssi
            if (offsetOnSSI < 100) {
               theMap->getItemCoordinates(closestID, 100,
                                          entryLat, entryLon);
               mc2dbg4 << "changed epLat and epLon, no unique streetsegment "
                       << poiSQLID << endl;
            } else if (offsetOnSSI > (65535-100)) {
               theMap->getItemCoordinates(closestID, 65535-100,
                                          entryLat, entryLon);
               mc2dbg4 << "changed epLat and epLon, no unique streetsegment " 
                       << poiSQLID << endl;
            }

            // Check if this POI already is represented in epData
            map<uint32,epData_t>::iterator it = epData.find(poiSQLID);
            if (it != epData.end()) {
               // Already present, update if distance smaller
               // e.g. if the POI is located close to a mcm map border the
               // first ep-dist was perhaps to a ssi in the wrong mcm map.
               if (it->second.dist_sqmc2 > closestSqDist_mc2) {
                  it->second.dist_sqmc2 = closestSqDist_mc2;
                  it->second.mapID = theMap->getMapID();
                  it->second.lat = entryLat;
                  it->second.lon = entryLon;
               }
            } else {
               // Not there -- insert
               epData_t d;
               d.dist_sqmc2 = closestSqDist_mc2;
               d.mapID = theMap->getMapID();
               d.lat = entryLat;
               d.lon = entryLon;
               epData[poiSQLID] = d;
            }

         } else {
            mc2dbg8 << "No closeID for " << poiSQLID << endl;
         }
     
      } else {
         mc2dbg4 << "POI with SQL-ID " << poiSQLID << " already have at "
                 << "least one (" << subSqlQuery->getColumn(0) 
                 << ") entry point(s)" << endl;
      }
      
   }
   //Check that no errors occured in sqlQuery
   if ( sqlQuery->getError() != 0 ) {
      mc2log << error << here << " Problem with sql query, error code="
             << sqlQuery->getError() << ", errror string='"
             << sqlQuery->getErrorString() << "'" << endl;
      MC2_ASSERT(false);
   }
   delete sqlQuery;
   delete subSqlQuery;

   mc2dbg << "After looping map " << theMap->getMapID() << " there are "
          << epData.size() << " entry point candidates in epData"
          << endl;

   return 0;
}


int
WASPExtractor::updateInMap(OldGenericMap* theMap, uint32 mapVersionID,
                           char* date, ofstream& outFile,
                           uint32& nbrNewPois, uint32& nbrModifiedPois,
                           uint32& nbrRemovedPois,
                           bool skipControlledAccessPOIs)
{
   if (!m_connected) {
      mc2log << fatal << "Not connected" << endl;
      return -1;
   }
   //Get boundingbox
   MC2BoundingBox bbox;
   theMap->getMapBoundingBox(bbox);
   //Get map id
   uint32 mapID = theMap->getMapID();
   mc2log << "Map id = " << mapID << endl;
   
   mc2log << "Map version id = " << mapVersionID 
          << " (from EDVersion), date = \"" << date << "\"" << endl;
   
   // If running dynamic WASPing on a first geneneration map, we
   // need to remove all original map POIs that do not have a link to WASP
   uint32 nbrRemovedFirstGenPOIs = 0;
   if ( theMap->getWaspTime() == 0 ) {
      mc2dbg1 << "First gen map, will remove all POIs from the map "
              << " before doing dynamic" << endl;
      nbrRemovedFirstGenPOIs = removeAllPOI(theMap);
      mc2log << "Removed " << nbrRemovedFirstGenPOIs 
             << " POIs from map " << theMap->getMapID() 
             << " before doing dynamic" << endl;
   }
   
   // If told to skip POIs with controlled access, use this sub query
   // when dealing with new and modified POIs
   // For the remove POIs, OK to deal with ctrl-acc-POIs, cause with this
   // option we already stated that we do not want them there so it is
   // ok to remove them if they are found with the remove-requirements
   char subControlledAccessWhereQuery[4096];
   if ( skipControlledAccessPOIs ) {
      sprintf(subControlledAccessWhereQuery, " AND rights = \"FFFFFFFFFFFFFFFF\" ");
   } else {
      sprintf(subControlledAccessWhereQuery, " ");
   }


   // ---------------------------
   // And query for new and modify.
   char andQuery[4096];
   sprintf(andQuery, "lastModified >= %s%s%s "
                     "AND country=%i "
                     "AND lat<%i AND lat>%i "
                     "AND (lon-%i)<0 and (lon-%i)>0 "
                     " %s"
                     "AND (validFromVersion <= %i "
                     "OR validFromVersion IS NULL) "
                     "AND (validToVersion >= %i "
                     "OR validToVersion IS NULL);",
                     "\"", date, "\"",
                     int(theMap->getCountryCode()),
                     bbox.getMaxLat(), bbox.getMinLat(),
                     bbox.getMaxLon(), bbox.getMinLon(),
                     subControlledAccessWhereQuery,
                     mapVersionID, mapVersionID);
   
   set<uint32>::iterator it;

   // Get user rights to fill in from map.
   UserRightsItemTable::itemMap_t userRights;
   int nbrRights = theMap->getUserRights( userRights );
   mc2log << info << "Number of rights in map 0x" << hex 
          << theMap->getMapID() << dec << ": " << nbrRights << endl;

   // If poi is new or modified
   // inUse=1 and deleted=0 and the validFrom- and ToVersions 
   // match the version of this map
   nbrNewPois = 0;
   nbrModifiedPois = 0;
   char newOrModQuery[4096];
   sprintf(newOrModQuery, "SELECT ID, lat, lon, rights FROM POIMain "
                        "WHERE inUse=1 AND deleted=0 "
                        "AND %s ",
                        andQuery);
   mc2log << "select new/modified query: " << newOrModQuery << endl;
   SQLQuery* sqlQuery = m_sqlConnection->newQuery();
   if ( ! doQuery(sqlQuery, newOrModQuery, "WASPExtractor::updateInMap") ) {
      delete sqlQuery;
      return -1;
   }
   uint32 nbrSelected = 0;
   while (sqlQuery->nextRow()) {
      nbrSelected++;
      uint32 newOrModPOIID = uint32(atoi(sqlQuery->getColumn(0)));
      int32 lat = atoi(sqlQuery->getColumn(1));
      int32 lon = atoi(sqlQuery->getColumn(2));      
      MC2String right(sqlQuery->getColumn(3));
      mc2dbg8 << "Raw right: " << right << endl;
      MapRights poiRights = MapRights( right );
      
      mc2dbg4 << "New or modified POI, waspid = " << newOrModPOIID << endl;
      it = m_addedIDs.find(newOrModPOIID);
      if ( it == m_addedIDs.end() ) {

         // Try to get the POI from this map.
         // if it exists in this map the POI has been modified,
         // if it does not exist in this map the POI is new and should be added
         OldPointOfInterestItem* newOrModPoi = 
            findPOIWithWASPID(theMap, newOrModPOIID);
         
         SQLQuery* subSqlQuery = m_subSqlConnection->newQuery();
         poiAddData_t notInserted;
         set<uint16> poiCategories;
         bool newPOI = false;
         if ( newOrModPoi != NULL) {
            // modify
         } else {
            // new, create a POI
            newOrModPoi = new OldPointOfInterestItem(MAX_UINT32);
            newPOI = true;
         }
            
         newOrModPoi = createPOI(theMap, subSqlQuery, newOrModPoi, 
                                 newOrModPOIID, lat, lon, 
                                 newPOI, notInserted, 
                                 poiCategories);

         if (newOrModPoi != NULL) {

            if ( newPOI ) {
               //Add new POI to map
               uint32 newItemID = theMap->addItem(newOrModPoi, 14);
               mc2log << "POI with waspid " << newOrModPOIID << " item id " 
                      << newItemID << " added to map " << mapID << endl;
               outFile << mapID << ";" << newItemID << endl;

               // Add the categories.
               newOrModPoi->addCategories(*theMap, poiCategories);
               nbrNewPois++;
            } else {
               // modified poi
               uint32 modItemID = newOrModPoi->getID();
               mc2log << "POI with wasp id " << newOrModPOIID << " item id "
                      << modItemID << " modified in map " << mapID << endl;
               outFile << mapID << ";" << modItemID << endl;

               // Update the categories of the POI
               newOrModPoi->removeCategories(*theMap);
               newOrModPoi->addCategories(*theMap, poiCategories);
               nbrModifiedPois++;
            }
            
            // mark this POI as handled
            m_addedIDs.insert(newOrModPOIID);
            
            // Store the rights value for coming addition.
            pair<UserRightsItemTable::itemMap_t::iterator, bool> insRes = 
               userRights.insert(make_pair(newOrModPoi->getID(), poiRights));
            if (!insRes.second){
               // This one already exists in the map, set its value.
               insRes.first->second = poiRights;
            }
         }
         // else the POIs entry point did not fit this map, or it
         // has no entry point at all
         delete subSqlQuery;

      } else {
         mc2dbg2 << "Failed to add/modify POI waspid=" << newOrModPOIID
                 << ", already added/modified." << endl;
      }
   }
   mc2log << "Number new pois = " << nbrNewPois << ", number modified pois = "
          << nbrModifiedPois << " (of " << nbrSelected << " candidates)"
          << " in map " << theMap->getMapID() << endl;
   //Check that no errors occured in sqlQuery
   if ( sqlQuery->getError() != 0 ) {
      mc2log << error << here << " Problem with sql query, error code="
             << sqlQuery->getError() << ", errror string='"
             << sqlQuery->getErrorString() << "'" << endl;
      MC2_ASSERT(false);
   }
   delete sqlQuery;



   // ---------------------------
   // If poi is removed
   // inUse=0 or deleted=1 or the validToVersion is prior to
   // the version of this map
   nbrRemovedPois = 0;

   char rmAndQuery[4096];
   sprintf(rmAndQuery, 
           "lastModified >= %s%s%s "
           "AND country=%i "
           "AND lat<%i AND lat>%i "
           "AND (lon-%i)<0 and (lon-%i)>0 "
           "AND (validFromVersion <= %i "
           "OR validFromVersion IS NULL);",
           "\"", date, "\"",
           int(theMap->getCountryCode()),
           bbox.getMaxLat(), bbox.getMinLat(),
           bbox.getMaxLon(), bbox.getMinLon(),
           mapVersionID );

   char removedQuery[4096];
   sprintf(removedQuery, 
           "SELECT ID FROM POIMain "
           "WHERE added < %s%s%s "
           "AND ( ( inUse=0 ) "
			  "      OR ( deleted=1 ) "
           "      OR ( ( validToVersion < %i ) AND "
           "           ( validToVersion IS NOT NULL ) ) ) "
           "AND %s ",
           "\"", date, "\"",
           mapVersionID, rmAndQuery);
   mc2log << "select removed query: " << removedQuery << endl;
   sqlQuery = m_sqlConnection->newQuery();
   if ( ! doQuery(sqlQuery, removedQuery, "WASPExtractor::updateInMap") ) {
      delete sqlQuery;
      return -1;
   }
   // Go through all candidates found in db, collect item ids and wasp ids
   // of the pois that should be removed from this mcm map
   UserRightsItemTable::itemMap_t::iterator urIter;
   nbrSelected = 0;
   set<uint32> itemIDsToRemove;
   set<uint32> waspIDsToRemove;
   while (sqlQuery->nextRow()) {
      nbrSelected++;
      uint32 removedPOIID = uint32(atoi(sqlQuery->getColumn(0)));
      mc2dbg8 << "Remove POI, waspid = " << removedPOIID << endl;
      it = m_addedIDs.find(removedPOIID);
      if ( it == m_addedIDs.end() ) {
         OldPointOfInterestItem* removedPoi = 
            findPOIWithWASPID(theMap, removedPOIID);
         if ( removedPoi != NULL ) {
            uint32 removedItemID= removedPoi->getID();
            itemIDsToRemove.insert(removedItemID);
            waspIDsToRemove.insert(removedPOIID);
            mc2log << "POI with wasp id " << removedPOIID 
                   << " item id " << removedItemID 
                   << " to be removed from map " << mapID << endl;
         }
      }
   }
   mc2log << "Collected " << itemIDsToRemove.size() 
          << "=" << waspIDsToRemove.size()
          << " pois to remove (of " << nbrSelected << " candidates)" 
          << " in map " << theMap->getMapID() << endl;
   //Check that no errors occured in sqlQuery
   if ( sqlQuery->getError() != 0 ) {
      mc2log << error << here << " Problem with sql query, error code="
             << sqlQuery->getError() << ", errror string='"
             << sqlQuery->getErrorString() << "'" << endl;
      MC2_ASSERT(false);
   }
   delete sqlQuery;
   // Delete the POIs from the map and remove from user rights
   mc2dbg << "To delete the collected POIs from the map" << endl;
   if ( theMap->removeItems(itemIDsToRemove) ) {
      for ( set<uint32>::const_iterator cit = itemIDsToRemove.begin();
            cit != itemIDsToRemove.end(); cit++ ) {
         uint32 removedItemID = *cit;
         outFile << mapID << ";" << removedItemID << endl;
         nbrRemovedPois++;
         // remove also from userRights
         urIter = userRights.find( removedItemID );
         if ( urIter != userRights.end() ) {
            userRights.erase(urIter);
         }
      }
      for ( set<uint32>::const_iterator cit = waspIDsToRemove.begin();
            cit != waspIDsToRemove.end(); cit++ ) {
         m_addedIDs.insert( *cit );
      }
   }
   mc2log << "Number removed pois = " << nbrRemovedPois
          << " (of " << nbrSelected << " candidates)" 
          << " in map " << theMap->getMapID() << endl;

   // Set the user rights collected above.
   mc2log << "Nbr userRights: " << userRights.size() << endl;
   theMap->setUserRights(userRights);

   // print summarized info and return.
   mc2log << info << "updateInMap for map " << mapID 
          << " nbr modified=" << nbrModifiedPois 
          << " new=" << nbrNewPois 
          << " removed=" << nbrRemovedPois 
          << " removedFirstGenPOIs=" << nbrRemovedFirstGenPOIs << endl;
   return ( nbrModifiedPois + nbrNewPois + 
            nbrRemovedPois + nbrRemovedFirstGenPOIs );
}

bool
WASPExtractor::cityCentreWithLocationAltName(
      OldPointOfInterestItem* poi, poiName_t checkName, OldGenericMap* theMap)
{
   if ( !poi->isPointOfInterestType(ItemTypes::cityCentre) )
      return false;
   
   if (checkName.poiNameType != ItemTypes::alternativeName)
      return false;

   mc2dbg8 << "checking city cente alternative name " << poi->getWASPID()
           << " " << checkName.name << endl;

   LangTypes::language_t poiNameLang = checkName.poiNameLang;
   if ( (poiNameLang == LangTypes::invalidLanguage) ||
        (poi->getNameWithType(
            ItemTypes::officialName, poiNameLang) == MAX_UINT32) ) {
      return false;
   }

   // Ok, this poi has an alternative name, and the language of 
   // the name is the same as the language of any of the official
   // names of the poi.

   // Check if the poi is located in a bua or municipal that has
   // this alternative name...
   OldItem* ssi = theMap->itemLookup(poi->getStreetSegmentItemID());
   uint32 munID = MAX_UINT32;
   uint32 buaID = MAX_UINT32;
   if (ssi != NULL) {
      munID = theMap->getRegionID(ssi, ItemTypes::municipalItem);
      buaID = theMap->getRegionID(ssi, ItemTypes::builtUpAreaItem);
   }
   OldItem* mun = theMap->itemLookup(munID);
   OldItem* bua = theMap->itemLookup(buaID);

   uint32 strIdx;
   if (bua != NULL) {
      if ( theMap->itemHasNameAndType(
               bua, poiNameLang, ItemTypes::invalidName,
               checkName.name, strIdx) ) {
         // The built-up area share this poiName.
         mc2log << "poi waspid=" << poi->getWASPID() 
                << " has bua name (bua=" << buaID <<  " " 
                << checkName.name << endl;
         return true;
      }
   }
   if ( mun != NULL ) {
      if ( theMap->itemHasNameAndType(
               mun, poiNameLang, ItemTypes::invalidName,
               checkName.name, strIdx) ) {
         // The municipal share this poiName.
         mc2log << "poi waspid=" << poi->getWASPID() 
                << " has municipal name (mun=" << munID <<  " " 
                << checkName.name << endl;
         return true;
      }
   }
   
   return false;
}

int
WASPExtractor::waspLab(OldGenericMap* theMap, MC2String function )
{

   if (!m_connected) {
      mc2log << fatal << "WASPExtractor::waspLab Not connected" << endl;
      return -1;
   }
   //Get boundingbox
   MC2BoundingBox bbox;
   theMap->getMapBoundingBox(bbox);
   //Get map id
   uint32 mapID = theMap->getMapID();
   mc2dbg << "WASPExtractor::waspLab Map id = " << mapID << endl;
   
   
   // Check if there are any POIs in the map with waspID within a
   // specific ID range (e.g. one POI source)
   if ( function == "checkIfPOIsWithinRange" ) {

      uint32 minID = 195191689;
      uint32 maxID = 207267674;
   
      uint32 nbrPOIsInMap = 0;
      set<uint32> itemIDsToRemoveRange;
      for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
            OldPointOfInterestItem* poi = 
               dynamic_cast<OldPointOfInterestItem*>(theMap->getItem(z, i));
            if ( poi != NULL ) {
               nbrPOIsInMap++;
               uint32 myWaspId = poi->getWASPID();
               // check if waspID within Prem IDs
               if ( (myWaspId >= minID) && (myWaspId <= maxID) ) {
                  itemIDsToRemoveRange.insert(poi->getID());
                  cout << "POI within range, waspID " << myWaspId << endl;
               }
            }
         }
      }
      mc2log << "Found " << itemIDsToRemoveRange.size() 
             << " range pois in map " << theMap->getMapID() << endl;
   }

   // Delete all POIs from one specific source from the map
   else if ( function == "removePOIsFromOneSource" ) {

      uint32 sourceID = 10; // some source

      // 1. select all POIs from this source that are within the 
      //    map bbox. Store the ids in a vector.
      // 2. Loop the POIs in the map, if the waspID is among the 
      //    selected POIs, remove from the map

      
      // 1.
      char query[4096];
      sprintf(query, "SELECT ID FROM POIMain "
                     "where source=%i"
                     " AND lat<%i AND lat>%i"
                     " AND (lon-%i)<0 and (lon-%i)>0;",
                     sourceID,
                     bbox.getMaxLat(), bbox.getMinLat(),
                     bbox.getMaxLon(), bbox.getMinLon());
      mc2dbg << "Select query: " << query << endl;
      SQLQuery* sqlQuery = m_sqlConnection->newQuery();
      if ( ! doQuery(sqlQuery, query, "WASPExtractor::waspLab") ) {
         delete sqlQuery;
         return -1;
      }
      set<uint32> selectedPOIs;
      uint32 nbrSelected = 0;
      while (sqlQuery->nextRow()) {
         nbrSelected++;
         uint32 waspID = uint32(atoi(sqlQuery->getColumn(0)));
         selectedPOIs.insert( waspID );
      }
      mc2log << info << "Selected " << nbrSelected
             << " POIs within map bbox in map " << mapID 
             << ", set.size=" << selectedPOIs.size() << endl;
      //Check that no errors occured in sqlQuery
      if ( sqlQuery->getError() != 0 ) {
         mc2log << error << here << " Problem with sql query, error code="
                << sqlQuery->getError() << ", errror string='"
                << sqlQuery->getErrorString() << "'" << endl;
         MC2_ASSERT(false);
      }
      delete sqlQuery;
      
      // 2.
      uint32 nbrRemoved = 0;
      set<uint32>::const_iterator it;
      for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
            OldPointOfInterestItem* poi = 
               dynamic_cast<OldPointOfInterestItem*>(theMap->getItem(z, i));
            if ( poi != NULL ) {

               uint32 myWaspId = poi->getWASPID();
               it = selectedPOIs.find( myWaspId );
               if ( it != selectedPOIs.end() ) {
                  // please remove this POI
                  nbrRemoved++;
                  mc2log << "Removing POI waspID=" << myWaspId << endl;
                  // remove, no need to update hash table
                  theMap->removeItem( poi->getID(), false); 
               }
            }
         }
      }
      mc2log << info << "Removed " << nbrRemoved << " POIs from map"
             << mapID << endl;
      if ( nbrRemoved > 0 ) {
         mc2log << info << "Saving map" << endl;

         // Fixme: Need to handle also user rights in this remove!!!
         //theMap->save();
      }
      return nbrRemoved;
   }
   
   return 0; // ok
}


int main(int argc, char* argv[]) 
{
   // The setting for connecting to the POI database WASP.
   MC2String mydb = PropertyHelper::get<MC2String>("POI_SQL_DATABASE").c_str();
   MC2String myhost = PropertyHelper::get<MC2String>("POI_SQL_HOST").c_str();
   MC2String myuser = PropertyHelper::get<MC2String>("POI_SQL_USER").c_str();
   MC2String mypwd = PropertyHelper::get<MC2String>("POI_SQL_PASSWORD").c_str();
   const char* database = mydb.c_str();
   const char* host = myhost.c_str();
   const char* username = myuser.c_str();
   const char* password = mypwd.c_str();


   // The char encoding used in the DB
   CharEncodingType::charEncodingType poiDBCharEnc = 
      CharEncodingType::UTF8;
   
   // The maximum allowed distance between the entry point of a POI 
   // and the street segment closest to the entry point, for the POI to be
   // added to the map in normal WASPing (addPOIToMap)
   uint32 maxDistMeter = 5;
   // This distance is also used for deciding if the POI will have a gfxData
   // or not; 
   // If the distance between the POI symbol coordinate (POIMain.lat lon) and
   // the entry point of the POI is less than maxDistMeter, the POI will have
   // no gfxData. If the distance between symbol coord and entry point is
   // larger than maxDistMeter, the POI will have a point gfxData.
   
   
   CommandlineOptionHandler coh(argc, argv, 1);
   char summaryStr[300];
   sprintf(summaryStr,
           "Use for adding POIs to mcm maps in normal or dynamic map "
           "generation, or for creating entry points for POIs in POI db.\n"
           "Connects to db: %s on: %s.",
           database, host );
   coh.setSummary( summaryStr );
   coh.setTailHelp("mcm-file(s)");

   //---------------------------------Add POI:s to Map, normal WASPing
   bool o_addPOIToMap = false;   
   coh.addOption("-a", "--addPOIToMap",
                 CommandlineOptionHandler::presentVal,
                 1, &o_addPOIToMap, "F",
                 "Main function\n"
                 "Adds POI:s to maps = normal WASPing. "
                 "The maps are cleaned from POIs before WASPing.\n"
                 "Possible to combine with --skipControlledAccessPOIs");
   
   //--------------------------------Update POIs in Map, dynamic WASPing
   bool o_updatePOIInMap = false;
   coh.addOption("-t", "--updatePOIInMap",
                 CommandlineOptionHandler::presentVal,
                 1, &o_updatePOIInMap, "F",
                 "Main function\n"
                 "Update POI:s in merged maps = dynamic WASPing. "
                 "All POIs that have been added or in some way modified "
                 "(including inUse set to 0 etc for removal) in the database "
                 "after the date of the last WASPing of the looped map "
                 "will be changed accordingly\n"
                 "Possible to combine with --skipControlledAccessPOIs");
   
   // --------------------------------- Create entrypoints
   bool o_createEP = false;
   coh.addOption("-c", "--createEntryPoints",
                 CommandlineOptionHandler::presentVal,
                 1, &o_createEP, "F",
                 "Main function\n"
                 "Loops the maps in tail and find POIs in WASP that would "
                 "fit in the maps in case of normal WASPing. "
                 "If the POIs have no entry point, this method "
                 "creates one. Inserts the entry points into WASP and "
                 "updates the lastModified date of the POI that got EP.\n"
                 "Howto:\n"
                 "First finding a ssi within 200 meters from "
                 "the POI symbol coordinate with the same name as the POI "
                 "visiting address, else any closest ssi to symbol coord.\n"
                 "\n"
                 "Must loop all underview maps of a country at the same time.\n"
                 "Combine with --source to specify which POI source to process.\n"
                 "If outfile is wanted (instead of direct insertion "
                 "of EPs into WASP), combine with --epToFile option which "
                 "will create and print "
                 "to file 'insertEntryPoints_sql_SOURCE_DATE_TIME.txt'.");

   // --------------------------------- Tool
   char* o_waspLab = NULL;
   coh.addOption("", "--waspLab",
                 CommandlineOptionHandler::stringVal,
                 1, &o_waspLab, "",
                 "What it does depends on compilation.\n"
                 "   removePOIsFromOneSource (hardcoded source id)\n"
                 "   checkIfPOIsWithinRange");

   // --------------------------------- Skip controlled access POIs
   bool o_skipControlledAccessPOIs = false;
   coh.addOption("", "--skipControlledAccessPOIs",
                 CommandlineOptionHandler::presentVal,
                 1, &o_skipControlledAccessPOIs, "F",
                 "Help option to --addPOIToMap and --updatePOIInMap.\n"
                 "Do not add or update POIs that have controlled acces,"
                 "i.e. the ones with POIMain.rights not FFFFFFFFFFFFFFFF");

   //------------------------------------ POI source
   char* o_source = NULL;
   coh.addOption("", "--source",
         CommandlineOptionHandler::stringVal,
         1, &o_source, "",
         "Help option to --createEntryPoints\n"
         "Only create entry points for POIs from this source.");

   //------------------------------------ Print ep to file
   bool o_epToFile = false;
   coh.addOption("", "--epToFile",
         CommandlineOptionHandler::presentVal,
         1, &o_epToFile, "F",
         "Help option to --createEntryPoints\n"
         "Print queries to file instead of insert into WASP.");



   // Parse command-line
   if(!coh.parse()) {
      cerr << argv[0] << ": Error on commandline! (-h for help)" << endl;
      exit(1);
   }


   WASPExtractor* waspExtr = new WASPExtractor(
         database, host, username, password, poiDBCharEnc, maxDistMeter);

   map<uint32, WASPExtractor::epData_t> epData;
   //If map should be updated, open outfile
   ofstream outFile ("modifiedPOIs.txt");
   uint32 totNbrNewPOIs = 0;
   uint32 totNbrModifiedPOIs = 0;
   uint32 totNbrRemovedPOIs = 0;
   // POI sql-ID, data
   typedef multimap<uint32, WASPExtractor::poiAddData_t> notAddedIDs_t;
   notAddedIDs_t notAddedIDs;


   
   // Loop for all maps, and add indata to them
   int nbrMaps = coh.getTailLength();
   for (int i=0; i<nbrMaps; i++) {
      const char* mcmName = coh.getTail(i);
      OldGenericMap* theMap = OldGenericMap::createMap(mcmName);

      if (theMap != NULL) {

         // Preparations
         // ---------------------------------
         
         // Close and open the connection to DB, not to risk 
         // lost connection to mysql during dynamic WASPing.
	      if (o_updatePOIInMap) {
	         waspExtr->closeThenOpenDatabaseConnection(
               database, host, username, password, poiDBCharEnc);
	      }

         // If adding or updating pois in map,
         // check that mapVersion is valid and get the map version id
         // (map version is check against POI.EDVersion)
         uint32 mapVersionID = 0;
         if ( o_addPOIToMap || o_updatePOIInMap ) {
	               
            const char* mapVersion = theMap->getMapOrigin();
            bool versionValid =  waspExtr->checkValidMapVersion( mapVersion, 
                                                          o_addPOIToMap, 
                                                          mapVersionID );
            if ( versionValid ){
               mc2log << info << "Map version \"" << mapVersion 
                      << "\" ok, id = " << mapVersionID << endl;
            } else {
               mc2log << fatal << "Invalid map version (" 
                      << mapVersion << ")" << endl;
               delete waspExtr;
               MC2_ASSERT(false);
            }
         }


         // Action
         // --------------------------------
         
         if ( o_waspLab != NULL ) {
            waspExtr->waspLab( theMap, o_waspLab );
         }
         
         else if (o_createEP) {
            if ( o_source == NULL ) {
               mc2log << fatal << "You must specify source" << endl;
               delete waspExtr;
               return -1;
            } else {
               
               int32 result = 
                  waspExtr->createEntryPoints(
                     theMap, epData, 
                     o_source );
               if ( result < 0 ) {
                  mc2dbg << "createEntryPoints failed - exit" << endl;
                  delete waspExtr;
                  MC2_ASSERT(false);
               }
            }
         }
         
         else if (o_addPOIToMap) {
            uint32 curWaspTime = TimeUtility::getRealTime();
            int status = waspExtr->addToMap(theMap, 
                                     notAddedIDs, 
                                     mapVersionID,
                                     o_skipControlledAccessPOIs);
            if (status < 0) {
               mc2log << fatal << "Failed to add POI:s to " << mcmName
                      << " (" << status << ")" << endl;
               MC2_ASSERT(false);
            } else {
               mc2log << info << "Added " << status << " POIs to " 
                      << mcmName  << endl;
               // Set wasp-date in the map header.
               theMap->setWaspTime(curWaspTime);
               char waspDate[126];
               waspDate[0] = '\0';
               waspExtr->getWaspDateFromMap(theMap, waspDate);
               mc2log << "For map " << theMap->getMapID() 
                      << " new waspdate \"" << waspDate << "\"" << endl;

               // Create and set admin area centres for municipals and
               // built up areas
               uint32 nbrAdminCentres = 
                  theMap->createAndSetCentreCoordinatesForAdminAreas();
               mc2log << info << "Added " << nbrAdminCentres
                      << " admin area centres to the map" << endl;
               
               // Some check
               uint32 nbrPOI = 0;
               uint32 nbrPOIGfx = 0;
               const uint32 z = 14;
               for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
                  OldPointOfInterestItem* poi = 
                     dynamic_cast<OldPointOfInterestItem*>
                     (theMap->getItem(z, i));
                  if ( poi != NULL) { 
                     ++nbrPOI;
                     if (poi->getGfxData() != NULL) 
                        ++nbrPOIGfx;
                  }
               } // for i
               mc2dbg << "Checked in map=" << theMap->getMapID() 
                      << ": #poi=" << nbrPOI << ",#poiNoGfx="
                      << nbrPOIGfx << endl;
               
               mc2dbg << "After map " << theMap->getMapID() 
                      << " addedIDs contains " << waspExtr->getNbrAddedIds()
                      << " elements." << endl;
            }

            // Save...
            theMap->save();
         }
         else if (o_updatePOIInMap) {
            // Get latest wasp-date from the map header
            char waspDate[126];
            waspDate[0] = '\0';
            bool waspDateFromMap = waspExtr->getWaspDateFromMap(theMap, waspDate);
            if ( !waspDateFromMap ) {
               mc2log << fatal << "No wasp-date given "
                      << "in map headers" << endl;
               delete waspExtr;
               return -1;
            }
            char* waspDateForMap = waspDate;
            
            mc2log << "Update POIs in map " << theMap->getMapID()
                   << " with waspTime " << waspDateForMap << endl;
            uint32 curWaspTime = TimeUtility::getRealTime();

            uint32 nbrNewPois = 0; uint32 nbrModifiedPois = 0;
            uint32 nbrRemovedPois = 0;
            int nbrUpdates = waspExtr->updateInMap( theMap, mapVersionID, 
                           waspDateForMap, outFile,
                           nbrNewPois, nbrModifiedPois, nbrRemovedPois,
                           o_skipControlledAccessPOIs);
            if (nbrUpdates < 0) {
               mc2log << "Failed to update POIs in map " 
                      << theMap->getMapID() << endl;
               MC2_ASSERT(false);
            }
            else if (nbrUpdates > 0) {
               // Pois changed in this map:
               // update admin area centres, set new wasp-date and save
               theMap->updateAdminAreaCentres();
               theMap->setWaspTime(curWaspTime);
               waspDateFromMap = waspExtr->getWaspDateFromMap(theMap, waspDate);
               mc2log << "For map " << theMap->getMapID() 
                      << " new waspdate \"" << waspDate << "\"" << endl;
               theMap->save();
               totNbrNewPOIs += nbrNewPois;
               totNbrModifiedPOIs += nbrModifiedPois;
               totNbrRemovedPOIs += nbrRemovedPois;
            }
         }
         else {
            mc2dbg1 << "Nothing todo!" << endl;
         
         }
         
         delete theMap;
      }
   }

   // ---------------------------------------------------------
   // Done looping the mcm maps.
   // Print the stuff that was collected during looping.
   
   if ( o_updatePOIInMap ) {
      mc2log << info << "Dynamic WASPing done looping " << nbrMaps
             << " maps." << endl;
      mc2log << info << " totNbrNewPOIs: " << totNbrNewPOIs << endl;
      mc2log << info << " totNbrModifiedPOIs: " << totNbrModifiedPOIs << endl;
      mc2log << info << " totNbrRemovedPOIs: " << totNbrRemovedPOIs << endl;
   }


   if ( o_waspLab != NULL ) {
      const map< MapRights, uint32 > poisPerRight = 
         waspExtr->getPOIsPerRight();
      map< MapRights, uint32 >::const_iterator it;
      for ( it = poisPerRight.begin();
            it != poisPerRight.end(); it++ ) {
         cout << "Total nbrPOIs with right " << it->first 
              << ": " << it->second << endl;
      }
   }


   if (o_createEP) {
   
      // Add the entry points to WASP or print insert-statements to file.
      // Also update lastModified.
      if ( epData.size() > 0 ) {
         mc2log << info << "Will try to add " << epData.size() 
                << " entry points to WASP/file" << endl;
         char dateStr[11];
         char timeStr[9];
         uint32 time = TimeUtility::getRealTime();
         StringUtility::makeDateStr( time, dateStr, timeStr );
         char fileName[256];
         if ( o_source != NULL ) {
            sprintf( fileName,
                     "insertEntryPoints_sql_%s_%s_%s.txt",
                     o_source, dateStr, timeStr);
         } else {
            sprintf( fileName,
                     "insertEntryPoints_sql_%s_%s.txt", dateStr, timeStr);
         }
         ofstream epFile ( fileName );

         SQLConnection* sqlConnection = 
            waspExtr->getSqlConnection();
         
         SQLQuery* sqlQuery = sqlConnection->newQuery(); 
         char queryText[4000];
         const char* epTaskDesc = "WASPExtractor::enterEP";
         const char* modTimeTaskDesc = "WASPExtractor::updatePoiModTime";
         
         // States
         uint32 queryState = 0; // All OK, insert EP to database.
         uint32 printState = 1; // Something failed, print rest of the queries.
         uint32 state = queryState;
         if ( o_epToFile ) {
            state = printState;
         }
         for (map<uint32, WASPExtractor::epData_t>::const_iterator
                 it=epData.begin(); it != epData.end(); ++it) {
                  
            uint32 waspID = it->first;
            // Insert EP query.
            sprintf(queryText,
                    "INSERT INTO POIEntryPoints (poiID, lat, lon) VALUES ("
                    "%i, %i, %i);", 
                    waspID,
                    it->second.lat,
                    it->second.lon);
            if ( state == queryState ){
               if ( ! waspExtr->doQuery(sqlQuery, queryText, epTaskDesc) ) {
                  delete sqlQuery;
                  state = printState; // Query failed, print this and the rest 
                  // of the queries to file.
                  mc2log << error << "Query: " << queryText << " failed."
                         << endl;
               }
            }
            if ( state == printState ){
               // Not else, since state might change in the if statement above.
               epFile << queryText << endl;
            }

            // Update POI modified time query
            sprintf(queryText,
                 "UPDATE POIMain set lastModified=NOW() where id = %i;", 
                 waspID);
            if ( state == queryState ){
               if ( ! waspExtr->doQuery(
                        sqlQuery, queryText, modTimeTaskDesc) ) {
                  delete sqlQuery;
                  state = printState; // Query failed, print this and
                  // the rest of the queries to file.
                  mc2log << error << "Query: " << queryText << " failed."
                         << endl;
               }
            }
            if ( state == printState ){
               // Not else, since state might change in the
               // if-statement above.
               epFile << queryText << endl;
            }
            
         }
         mc2log << info << "Handled " << epData.size() 
                << " entry points." << endl;

         if ( ! o_epToFile ) {
            if (state == printState) {
               // This indicates database faliure
               mc2log << info << "Wrote remaining entry point queries to "
                      << fileName << endl;
               mc2log << "Exits due to database faliure." << endl;
               exit(1);
            } else {
               // ok
               mc2log << info << "Added " << epData.size()
                      << " entry points to WASP" << endl;
            }
         } else {
            // Queries written to file
            mc2log << info << "Wrote sql queries for " << epData.size()
                   << " entry points to file " << fileName << endl;
         }
      
      } else {
         mc2log << info << "Did not create any entry points" << endl;
      }
   }
   
   else if (o_addPOIToMap) {

      // Dump information about the POI:s that were not added to
      // the looped maps during normal WASPing.
      // This will detect if there are POIs that need a new entry point
      // for the looped mapVersion, i.e. old POIs that had entry points 
      // created for an older mapVersion 
      // Alt. if the WASPing was incorrectly not done on all mcm maps 
      // of a country at the same time..
      
      // Remove POI:s that are added from notAddedIDs
      set<uint32> addedIDs;
      waspExtr->getAddedIds( addedIDs);
      for (set<uint32>::const_iterator it = addedIDs.begin();
                       it != addedIDs.end(); ++it) {
         notAddedIDs_t::iterator lower = notAddedIDs.lower_bound(*it);
         if (lower != notAddedIDs.end()) {
            notAddedIDs_t::iterator upper = notAddedIDs.upper_bound(*it);
            notAddedIDs.erase(lower, upper);
         }
      }
      
      // Dump all poi's that are not added
      // These POIs need a new entry point to fit the map!!
      // (or the WASPing was not done on all maps at the same time...)
      notAddedIDs_t::iterator iter=notAddedIDs.begin(); 
      while (iter != notAddedIDs.end()) {
         // Make sure that we use the ssi with the closest distance
         notAddedIDs_t::iterator lower = notAddedIDs.lower_bound((*iter).first);
         notAddedIDs_t::iterator upper = notAddedIDs.upper_bound((*iter).first);
         iter = lower;
         for (notAddedIDs_t::iterator tmpIT=lower; tmpIT!=upper; ++tmpIT) {
            mc2dbg4 << "Checking POI " << (*iter).first << ", dist="
                    << (*iter).second.dist_meters << ", "
                    << (*iter).second.streetName << endl;
            if ((*tmpIT).second.dist_meters < (*iter).second.dist_meters) {
               iter = tmpIT;
            }
         }

         mc2dbg << "POI " << (*iter).first << " not added. " 
                << (*iter).second.dist_meters << "m to closest street," 
                << (*iter).second.streetName << ", map=" 
                << (*iter).second.mapID << " (" 
                << (*iter).second.lat << "," << (*iter).second.lon 
                << ")" << endl;
         // This is the query to use if you want to add a new entry point
         // for the POI in the POI database.
         //cout <<"INSERT INTO POIEntryPoints (poiID, lat, lon) VALUES (" 
         //     << (*iter).first << ", " << (*iter).second.lat << "," 
         //     << (*iter).second.lon << ");" << endl;

         iter = upper;
      }
   }
   

   delete waspExtr;
}

