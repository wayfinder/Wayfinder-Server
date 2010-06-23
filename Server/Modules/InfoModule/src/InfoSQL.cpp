/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "InfoSQL.h"
#include "CharEncSQLConn.h"
#include "NonStdStl.h"
#include <sstream>
#include <memory>
#include <functional>
#include "DeleteHelpers.h"

#define ISQL "[ISQL:" << __LINE__ << "] "

InfoSQL::InfoSQL() :
   m_sqlConnection(NULL)
{   
   if(!setupDatabaseConnection()){
      exit(1); //a bit harsh. 
   }

   // populate the table of tables
   initTables();
   
   // Check database
   if ( ! initialCheckDatabase()) {
      mc2log << fatal << ISQL << "Initial database test: [FAILED]" << endl;
      exit(1);
   } else {
      mc2log << info << ISQL << "Initial database test: [SUCCESS]" << endl;
   };
}


InfoSQL::~InfoSQL() {
   delete m_sqlConnection;
}

bool InfoSQL::setupDatabaseConnection()
{   
   delete m_sqlConnection;
   m_sqlConnection = NULL;

   // get parameters using mc2.prop
   const char* driverName   = Properties::getProperty("INFO_SQL_DRIVER");
   const char* sqlHost      = Properties::getProperty("INFO_SQL_HOST");
   const char* sqlDB        = Properties::getProperty("INFO_SQL_DATABASE");
   const char* sqlUser      = Properties::getProperty("INFO_SQL_USER");
   const char* sqlPasswd    = Properties::getProperty("INFO_SQL_PASSWORD");
   const char* tmpSqlChEnc  = Properties::getProperty("INFO_SQL_CHARENCODING");
  
   MC2String sqlChEnc = "ISO-8859-1";
   if ( tmpSqlChEnc != NULL ){
      sqlChEnc = tmpSqlChEnc;
   }
      
   if (NULL == driverName || NULL == sqlHost || NULL == sqlDB ||
       NULL == sqlUser || NULL == sqlPasswd) {
      mc2log << fatal << ISQL << "one or more of the properties: "
             << "INFO_SQL_DRIVER, INFO_SQL_HOST, INFO_SQL_DATABASE, "
             << "INFO_SQL_USER or INFO_SQL_PASSWORD is missing!" << endl;
      return false;
   }

   auto_ptr<SQLDriver> driver;
   if (strcmp(driverName, "mysql") == 0) {
      driver.reset( new MySQLDriver(sqlHost, sqlDB, sqlUser, sqlPasswd) );
   } else if (strcmp(driverName, "mysqlrepl") == 0) {
      driver.reset( new MySQLReplDriver(sqlHost, sqlDB, sqlUser, sqlPasswd) );
   } else if(strcmp(driverName, "postgresql") == 0) {
      driver.reset( new PostgreSQLDriver(sqlHost, sqlDB, sqlUser, sqlPasswd) );
#ifdef USE_ORACLE
   } else if(strcmp(driverName, "oracle") == 0) {
      driver.reset( new OracleSQLDriver(sqlHost, sqlDB, sqlUser, sqlPasswd) );
#endif
   } else {
      mc2log << fatal << ISQL << "Unknown database driver: '" << driverName
             << "' specified in mc2.prop!" << endl;
      return false;
   }
   
   mc2log << info << ISQL << "Connecting as " << sqlUser << " to " << sqlDB
          << "@" << sqlHost << " using the " << driverName << " driver." <<endl;
   
   const CharEncodingType::charEncodingType infoDBChEnc =
      CharEncoding::encStringToEncType( sqlChEnc );
   const CharEncodingType::charEncodingType mc2ChEnc = 
      CharEncoding::getMC2CharEncoding();
   mc2log << info << ISQL << "DB char encoding:" 
          << CharEncoding::encTypeToEncString(infoDBChEnc) << endl;
   mc2log << info << ISQL << "MC2 char encoding:" 
          << CharEncoding::encTypeToEncString(mc2ChEnc) << endl;

   m_sqlConnection = new CharEncSQLConn(driver.get(), infoDBChEnc, mc2ChEnc);
   driver.release();

   if ( !m_sqlConnection->connect() ) {
      mc2log << fatal << ISQL << "InfoSQL couldn't connect to SQL database! "
             << "Aborting!" << endl;
      return false;
   }
   return true;
}

bool
InfoSQL::checkDatabaseConnection(int maxNbrTries)
{
   if( m_sqlConnection && m_sqlConnection->ping()){
      return true;
   }

   mc2log << warn << ISQL << "No connection to DB, trying to connect..."<<endl;
   
   for(int n = 0; n < maxNbrTries; ++n){
      if( setupDatabaseConnection() ){
         mc2log << info << ISQL << "DB Connection reestablished." << endl;
         return true;
      }
   }
   return false;
}

bool
InfoSQL::addDisturbance(DisturbanceElement* distElem)
{
   if( ! checkDatabaseConnection() ) {
      return false;
   }
   mc2dbg << ISQL << "InfoSQL::addDisturbance" << endl;
   bool newDisturbance = false;
   if( (distElem->getDisturbanceID() == MAX_UINT32) ) {
      const uint32 disturbanceID = getNewUniqueID("ISABDisturbance",
                                                  "disturbanceID");
      mc2dbg << ISQL << "InfoSQL new distID: " << disturbanceID << endl;
      if( disturbanceID == 0 ){
         return false;
      }
      distElem->setDisturbanceID(disturbanceID);
      newDisturbance = true;
   } else {
      mc2dbg << ISQL << "old distID: " << distElem->getDisturbanceID() << endl;
   }
   
   const uint32 disturbanceID = distElem->getDisturbanceID();
   const MC2String situationReference = distElem->getSituationReference();
   const TrafficDataTypes::disturbanceType type = distElem->getType();
   const TrafficDataTypes::phrase phrase = distElem->getPhrase();
   const uint32 eventCode = distElem->getEventCode();
   const uint32 startTime = distElem->getStartTime();
   const uint32 endTime = distElem->getEndTime();
   const uint32 creationTime = distElem->getCreationTime();
   const TrafficDataTypes::severity severity = distElem->getSeverity();
   const TrafficDataTypes::direction direction = distElem->getDirection();
   MC2String text = StringUtility::SQLEscapeSecure(distElem->getText());
   const bool deleted = distElem->getDeleted();
   MC2String firstLocation = distElem->getFirstLocation();
   MC2String secondLocation = distElem->getSecondLocation();
   const uint32 extent = distElem->getExtent();
   const uint32 costFactor = distElem->getCostFactor();
   const uint32 queueLength = distElem->getQueueLength();
   
   // Makes a CharEncSQLQuery, and saves all the parameters in the database
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   char query[4096];
   if(text.size()> 2000)
      text.erase(1900);
   if(firstLocation.size()> 50)
      firstLocation.erase(29);
   if(secondLocation.size()> 50)
      secondLocation.erase(29);
   if(situationReference.size()> 50){
      mc2log << error
             << "[InfoSQL::addDisturbance] situationReference too long ("
             << situationReference.size() << "):"
             <<  situationReference.c_str() << endl
             << "This cant be right, ignoring " << endl;
      return false;
   }
   sprintf(query, "REPLACE INTO ISABDisturbance(disturbanceID, "
           "situationReference, "
           "type, phrase, eventCode, startTime, endTime, creationTime, "
           "severity, direction, text, deleted, firstLocation, "
           "secondLocation, extent, costFactor, queueLength) "
           "VALUES(%d, '%s', %d, %d, %d, %d, %d, %d, %d, %d, "
           "'%s', %d, '%s', '%s', %d, %d, %d)",
           disturbanceID, situationReference.c_str(),
           type, phrase,
           eventCode, startTime, endTime, creationTime, severity,
           direction, text.c_str(), deleted, firstLocation.c_str(),
           secondLocation.c_str(), extent, costFactor, queueLength);
   if ( !doQuery(sqlQuery, query, "InfoSQL::addDisturbances()") ) {
      if(newDisturbance){
         mc2log << error << "InfoSQL: addDisturbances() INSERT failed!"
                << endl;
      } else {
         mc2log << error << "InfoSQL: addDisturbances() UPDATE failed!"
                << endl;
      }
      
      delete sqlQuery;
      return false;
   }

   // ********* collect all coords into one query.
   map<uint32, int32> latVector = distElem->getLatMap();
   map<uint32, int32> lonVector = distElem->getLonMap();
   map<uint32, uint32> angleVector = distElem->getAngle();
   vector<uint32> indexVector = distElem->getRouteIndex();
   
   MC2String queryString = "REPLACE INTO ISABDisturbanceCoords(disturbanceID, "
      "situationReference, latitude, longitude, angle, routeIndex) VALUES";
   //CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   
   for(vector<uint32>::iterator indexIt = indexVector.begin(); 
       indexIt != indexVector.end();
       ++indexIt)
   {
      char query[4096];
      uint32 routeIndex = *indexIt;
      map<uint32, int32>::iterator latlonIt = latVector.find(routeIndex);
      int32 latitude = latlonIt->second;
      latlonIt = lonVector.find(routeIndex);
      int32 longitude = latlonIt->second;
      map<uint32, uint32>::iterator it = angleVector.find(routeIndex);
      uint32 angle = it->second;
      sprintf(query, "(%d, '%s', %d, %d, %d, %d),",
              disturbanceID, situationReference.c_str(), latitude,
              longitude, angle, routeIndex);
      queryString += query;
   }
   //remove the last ',' from the query string
   {
      MC2String::size_type comma = queryString.find_last_of(',');
      if(comma != MC2String::npos){
         queryString.erase(comma, MC2String::npos);
      }
   }

   if ( !doQuery(sqlQuery,
                 queryString.c_str(), "InfoSQL::addDisturbances()") ) {
      if(newDisturbance){
         mc2log << error << "InfoSQL:addDisturbancesCoords() INSERT failed!"
                << endl;
      } else {
         mc2log << error << "InfoSQL:addDisturbancesCoords() REPLACE failed!"
                << endl;
      }
      delete sqlQuery;      
      return false;
   }
   delete sqlQuery;
   return true;
}

bool
InfoSQL::getDisturbancesWithinBBox(const MC2BoundingBox& bbox,
                                   vector<DisturbanceElement*> &distVect)
{
   mc2dbg2 << ISQL << "Loading disturbances from SQL-database." << endl;
   if (! checkDatabaseConnection() ){
      return false;
   }
   const int32 maxLat = bbox.getMaxLat();
   const int32 minLat = bbox.getMinLat();
   const int32 maxLon = bbox.getMaxLon();
   const int32 minLon = bbox.getMinLon();
   
   char query[4096];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   
   sprintf(query, "SELECT * FROM ISABDisturbance, ISABDisturbanceCoords "
           "WHERE ISABDisturbanceCoords.latitude > %d AND "
           "ISABDisturbanceCoords.latitude < %d "
           "AND ISABDisturbanceCoords.longitude > %d AND "
           "ISABDisturbanceCoords.longitude < %d "
           "AND ISABDisturbance.disturbanceID = ISABDisturbanceCoords.disturbanceID",
           minLat, maxLat, minLon, maxLon ); 
   if( !doQuery(sqlQuery, query,
                "InfoSQL::getAllDisturbancesWithinBoundingbox()"))
   {
      mc2log << error << "InfoSQL::getAllDisturbancesWithinBoundingbox():"
             << " failed, returning false!" << endl;
      delete sqlQuery;
      return false;
   }
   uint32 disturbanceID = 0;
   MC2String situationReference = "";
   TrafficDataTypes::disturbanceType type;
   TrafficDataTypes::phrase phrase;
   uint32 eventCode = 0;
   uint32 startTime = 0;
   uint32 endTime = 0;
   uint32 creationTime = 0;
   TrafficDataTypes::severity severity;
   TrafficDataTypes::direction direction;
   MC2String firstLocation = "";
   MC2String secondLocation = "";
   uint32 extent = 0;
   uint32 costFactor = 0;
   uint32 queueLength= 0;
   MC2String text = "";
   bool deleted = false;
   
   DisturbanceElement* distElem = NULL;
   
   uint32 prevDisturbanceID = MAX_UINT32;
   while( sqlQuery->nextRow() ) {
      disturbanceID = uint32(atoi(sqlQuery->getColumn(0)));
      if( disturbanceID != prevDisturbanceID )
      {
         if( (distElem != NULL) && (prevDisturbanceID != MAX_UINT32) ) {
            if( distElem->getNbrCoordinates() > 0 ) {
               distVect.push_back(distElem);
            }
         }
         situationReference = sqlQuery->getColumn(1);
         type =
            TrafficDataTypes::disturbanceType(atoi(sqlQuery->getColumn(2)));
         phrase = TrafficDataTypes::phrase(atoi(sqlQuery->getColumn(3)));
         eventCode = uint32(atoi(sqlQuery->getColumn(4)));
         startTime = uint32(atoi(sqlQuery->getColumn(5)));
         endTime = uint32(atoi(sqlQuery->getColumn(6)));
         creationTime = uint32(atoi(sqlQuery->getColumn(7)));
         severity = TrafficDataTypes::severity(atoi(sqlQuery->getColumn(8)));
         direction =
            TrafficDataTypes::direction(atoi(sqlQuery->getColumn(9)));
         text = sqlQuery->getColumn(10);
         deleted = bool(atoi(sqlQuery->getColumn(11)));
         firstLocation = sqlQuery->getColumn(12);
         secondLocation = sqlQuery->getColumn(13);
         extent = uint32(atoi(sqlQuery->getColumn(14)));
         //it's amazing that this reads MAX_UINT32 even when the
         //database states '-1'.
         costFactor = uint32(atoi(sqlQuery->getColumn(15)));
         queueLength = uint32(atoi(sqlQuery->getColumn(16)));
         
        
         
         distElem =
            new DisturbanceElement(disturbanceID,
                                   situationReference,
                                   type,
                                   phrase,
                                   eventCode,
                                   startTime,
                                   endTime,
                                   creationTime,
                                   severity,
                                   direction,
                                   firstLocation,
                                   secondLocation,
                                   extent,
                                   costFactor,
                                   text,
                                   queueLength);
         distElem->setDeleted(deleted);
      } 
      int32 lat;
      int32 lon;
      uint32 angle;
      uint32 routeIndex;
      lat = int32(atoi(sqlQuery->getColumn(19)));
      lon = int32(atoi(sqlQuery->getColumn(20)));
      angle = uint32(atoi(sqlQuery->getColumn(21)));
      routeIndex = uint32(atoi(sqlQuery->getColumn(22)));
      
      distElem->addCoordinate(lat,
                              lon,
                              angle,
                              routeIndex);
      
      prevDisturbanceID = disturbanceID;
   }
   if( distElem != NULL ) {
      if( distElem->getNbrCoordinates() > 0 ) {
         distVect.push_back(distElem);
      } else {
         delete distElem;
      }
   }
   delete sqlQuery;
   
   mc2dbg4 << "[InfoSQL] Returning " << distVect.size() << " disturbances." << endl;
   
   return true;
}

bool
InfoSQL::getDisturbancesWithinRadius(int32 latitude,
                                     int32 longitude,
                                     uint32 distance,
                                     vector<DisturbanceElement*> &distVect)
{
   mc2dbg2 << "[InfoSQL] Loading disturbances from SQL-database." << endl;
   
   int32 maxLat = latitude  + distance;
   int32 minLat = latitude  - distance;
   int32 maxLon = longitude + distance;
   int32 minLon = longitude - distance;
   const MC2BoundingBox bbox(maxLat, minLon, minLat, maxLon);

   return getDisturbancesWithinBBox(bbox, distVect);
}

void 
InfoSQL::getDisturbancesForSupplier( vector<DisturbanceElement*> &distVect, 
                                     const MC2String& supplier, 
                                     const vector<MC2String>& toBeKept )
{
   getDisturbancesForSupplier(distVect, supplier);
   for(vector<MC2String>::const_iterator it = toBeKept.begin();
       it != toBeKept.end(); ++it){
      mc2dbg4 << "getDistForSupp ref: " << *it << endl;
      vector<DisturbanceElement*>::iterator distIt = distVect.begin();
      while ( distIt != distVect.end() ) {
         if ( *it == (*distIt)->getSituationReference() ) {
            mc2dbg8 << "getDistForSupp dist: " 
                    << (*distIt)->getSituationReference() << " : " 
                    << *distIt << endl;
            delete *distIt;
            distVect.erase(distIt);
         } else {
            ++distIt;
         }
      }
   }
}

void 
InfoSQL::getDisturbancesForSupplier( vector<DisturbanceElement*> &distVect,
                                     const MC2String& supplier )
{
   if (! checkDatabaseConnection() ){
      return;
   }
   //the query
   auto_ptr<CharEncSQLQuery> sqlQuery ( m_sqlConnection->newQuery() );

   //the query string
   MC2String s = "SELECT * FROM ISABDisturbance, ISABDisturbanceCoords WHERE "
      "ISABDisturbanceCoords.disturbanceID = ISABDisturbance.disturbanceID "
      "AND ISABDisturbance.situationReference LIKE '";
   s += supplier + "%'";
   
   if ( doQuery(sqlQuery.get(), s.c_str(),
                "InfoSQL::getDisturbancesForSupplier()") ) {
      map<uint32,DisturbanceElement*> disturbances;
      extractDisturbances(*(sqlQuery.get()), disturbances);
      for(map<uint32,DisturbanceElement*>::iterator it = disturbances.begin();
          it != disturbances.end(); ++it){
         distVect.push_back( it->second );
      }
   } else {
      mc2log << error << "InfoSQL:getDisturbancesForSupplier() "
             << "query failed: '" << s << "'" << endl;
   }
}

bool
InfoSQL::deleteDisturbances(uint32 disturbanceID,
                            const MC2String& situationReference,
                            const MC2String& supplier,
                            bool removeAll,
                            vector<MC2String>& toBeKept)
{
   if (! checkDatabaseConnection() ){
      return false;
   }

   MC2String query;
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   char tmp[1024];
   if( !removeAll ) {
      query = "DELETE FROM ISABDisturbance WHERE disturbanceID = ";
      sprintf(tmp, "%d", disturbanceID);
      query += tmp;
      query += " OR situationReference = '";
      query += situationReference;
      query += "'";
      if ( !doQuery(sqlQuery, query.c_str(), "InfoSQL::deleteDisturbances()") )
      {
         mc2log << error << "InfoSQL:deleteDisturbances() "
                << "DELETE failed!" << endl;
         delete sqlQuery;
         return false;
      }
      delete sqlQuery;
      sqlQuery = m_sqlConnection->newQuery();
      query = "";
      query = "DELETE FROM ISABDisturbanceCoords WHERE disturbanceID = ";
      sprintf(tmp, "%d", disturbanceID);
      query += tmp;
      query += " OR situationReference = '";
      query += situationReference;
      query += "'";
      
      if ( !doQuery(sqlQuery, query.c_str(), "InfoSQL::deleteDisturbances()") )
      {
         mc2log << error << "InfoSQL:deleteDisturbances() "
                << "DELETE failed!" << endl;
         delete sqlQuery;
         return false;
      }
      
      delete sqlQuery;
      return true;      
   } else {
      query = "DELETE FROM ISABDisturbance WHERE situationReference NOT IN(";
      for(uint32 i = 0; i < toBeKept.size(); i++) {
         query += "'";
         query += toBeKept[i];
         query += "'";
         if( i != (toBeKept.size() - 1) )
            query += ", ";
      }
      if(toBeKept.empty()){
         query += "'' ";
      }
      
      query += ") AND situationReference LIKE '";
      query += supplier;
      query += "%'";
      if ( !doQuery(sqlQuery, query.c_str(),
                    "InfoSQL::deleteDisturbances()") )
      {
         mc2log << error << "InfoSQL:deleteDisturbances() "
                << "DELETE failed!" << endl;
         delete sqlQuery;
         return false;
      }
      query = "";
      delete sqlQuery;
      sqlQuery = m_sqlConnection->newQuery();
      query = "DELETE FROM ISABDisturbanceCoords WHERE situationReference "
         "NOT IN(";
      for(uint32 i = 0; i < toBeKept.size(); i++) {
         query += "'";
         query += toBeKept[i];
         query += "'";
         if( i != (toBeKept.size() - 1) )
            query += ", ";
      }
      if(toBeKept.empty()){
         query += "'' ";
      }

      query += ") AND situationReference LIKE '";
      query += supplier;
      query += "%'";
      if ( !doQuery(sqlQuery, query.c_str(),
                    "InfoSQL::deleteDisturbances()") )
      {
         mc2log << error << "InfoSQL:deleteDisturbances() "
                << "DELETE failed!" << endl;
         delete sqlQuery;
         return false;
      }
      delete sqlQuery;
   }
   return true;
} 


bool
InfoSQL::updateDisturbance(uint32 disturbanceID,
                           TrafficDataTypes::disturbanceType type,
                           TrafficDataTypes::severity severity,
                           uint32 costFactor,
                           uint32 startTime,
                           uint32 endTime,
                           MC2String text)
{
   if (! checkDatabaseConnection() ){
      return false;
   }
   char query[4096];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   
   sprintf(query, "UPDATE ISABDisturbance SET type = %d, "
           "severity = %d, costFactor = %d, "
           "startTime = %d, endTime = %d, "
           "text = '%s' WHERE "
           "disturbanceID = %d",
           type, severity, costFactor, startTime, endTime,
           text.c_str(), disturbanceID );
   
   if( !doQuery(sqlQuery, query, "InfoSQL: updateDisturbances()") )
   {
      mc2log << error << "InfoSQL: updateDisturbances() UPDATE failed!"
             << endl;
      delete sqlQuery;
      return false;
   }
   
   delete sqlQuery;
   return true;
}


bool
InfoSQL::deleteOldDisturbances()
{
   if (! checkDatabaseConnection() ){
      return false;
   }
   uint32 timeNow = uint32(time(0));
   char query[4096];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   bool res = true;
   
   vector<uint32> IDs;
   sprintf(query, "SELECT disturbanceID FROM ISABDisturbance WHERE "
           "endTime <= %d AND endTime > 0 AND deleted = 1",
           timeNow);
   if( !doQuery(sqlQuery, query, "InfoSQL:deleteOldDisturbances()")) {
      mc2log << error << "InfoSQL::deleteOldDisturbances(): failed!"
             << endl;
      res = false;
   }
   while( sqlQuery->nextRow() ) {
      uint32 disturbanceID = atoi(sqlQuery->getColumn(0));
      IDs.push_back(disturbanceID);
   }
   delete sqlQuery;

   vector<uint32>::iterator it;
   for(it = IDs.begin(); it != IDs.end(); it++) {
      uint32 disturbanceID = *it;
      
      sqlQuery = m_sqlConnection->newQuery();
      sprintf(query, "DELETE FROM ISABDisturbance WHERE disturbanceID = %d ",
              disturbanceID);
      if ( !doQuery(sqlQuery, query, "InfoSQL::deleteOldDisturbances()")) {
         mc2log << error << "InfoSQL::deleteOldDisturbances(): failed!"
                << endl;
         res = false;
      }
      delete sqlQuery;

      sqlQuery = m_sqlConnection->newQuery();
      sprintf(query, "DELETE FROM ISABDisturbanceCoords WHERE disturbanceID = %d ",
              disturbanceID);
      if ( !doQuery(sqlQuery, query, "InfoSQL::deleteOldDisturbances()")) {
         mc2log << error << "InfoSQL::deleteOldDisturbances(): failed!"
                << endl;
         res = false;
      }
      delete sqlQuery;
   }
   sqlQuery = m_sqlConnection->newQuery();
   sprintf(query, "UPDATE ISABDisturbance SET deleted = %d "
           "WHERE endTime <= %d AND endTime > %d", 
           1, timeNow, 0 );
   if ( !doQuery(sqlQuery, query, "IS::deleteOldDisturbances()")) {
      mc2log << error << "IS::deleteOldDisturbances(): failed!" << endl;
      res = false;   
   } 
   delete sqlQuery;
   return res; 
}

void
InfoSQL::extractDisturbances(SQLQuery& sqlQuery, 
                             map<uint32,DisturbanceElement*>& disturbances)
{   
   while( sqlQuery.nextRow() ) {
      uint32 disturbanceID = uint32(atoi(sqlQuery.getColumn(0)));

      DisturbanceElement* element = NULL;
      if ( disturbances.find(disturbanceID) != disturbances.end() ) {
         element = disturbances[disturbanceID];
      }
      if(! element ) {         
         //extract the data
         MC2String situationReference = sqlQuery.getColumn(1);
         TrafficDataTypes::disturbanceType type =
            TrafficDataTypes::disturbanceType(atoi(sqlQuery.getColumn(2)));
         TrafficDataTypes::phrase phrase = 
            TrafficDataTypes::phrase(atoi(sqlQuery.getColumn(3)));
         uint32 eventCode    = uint32(atoi(sqlQuery.getColumn(4)));
         uint32 startTime    = uint32(atoi(sqlQuery.getColumn(5)));
         uint32 endTime      = uint32(atoi(sqlQuery.getColumn(6)));
         uint32 creationTime = uint32(atoi(sqlQuery.getColumn(7)));
         TrafficDataTypes::severity severity = 
            TrafficDataTypes::severity(atoi(sqlQuery.getColumn(8)));
         TrafficDataTypes::direction direction =
            TrafficDataTypes::direction(atoi(sqlQuery.getColumn(9)));
         MC2String text           = sqlQuery.getColumn(10);
         bool deleted       = bool(atoi(sqlQuery.getColumn(11)));
         MC2String firstLocation  = sqlQuery.getColumn(12);
         MC2String secondLocation = sqlQuery.getColumn(13);
         uint32 extent      = uint32(atoi(sqlQuery.getColumn(14)));
         uint32 costFactor  = uint32(atoi(sqlQuery.getColumn(15)));
         uint32 queueLength = uint32(atoi(sqlQuery.getColumn(16)));
         
         //create the DisturbanceElement
         element = 
            new DisturbanceElement(disturbanceID,
                                   situationReference,
                                   type,
                                   phrase,
                                   eventCode,
                                   startTime,
                                   endTime,
                                   creationTime,
                                   severity,
                                   direction,
                                   firstLocation,
                                   secondLocation,
                                   extent,
                                   costFactor,
                                   text,
                                   queueLength);
         element->setDeleted(deleted);
         disturbances[disturbanceID] = element;
      } 
      //read the coordinate for this line
      int32  lat        =  int32(atoi(sqlQuery.getColumn(19)));
      int32  lon        =  int32(atoi(sqlQuery.getColumn(20)));
      uint32 angle      = uint32(atoi(sqlQuery.getColumn(21)));
      uint32 routeIndex = uint32(atoi(sqlQuery.getColumn(22)));
      //and add to the disturbance.
      element->addCoordinate(lat, lon, angle, routeIndex);
   }
}

DisturbanceElement* InfoSQL::getDisturbance(uint32 disturbanceID,
                                            const MC2String& situationReference)
{
   if (! checkDatabaseConnection() ){
      return false;
   }
   DisturbanceElement* foundDist = NULL;
   auto_ptr<CharEncSQLQuery> sqlQuery ( m_sqlConnection->newQuery() );

   std::ostringstream query_stream;

   query_stream << "SELECT * FROM ISABDisturbance, ISABDisturbanceCoords WHERE ";
   if(disturbanceID != MAX_UINT32){
      query_stream << "ISABDisturbance.disturbanceID = " << disturbanceID 
                   << " AND ";
   }
   if(situationReference != ""){
      query_stream << "ISABDisturbance.situationReference = \"" 
                   << situationReference << "\" AND ";
   }
   query_stream << "ISABDisturbance.disturbanceID = " 
                << "ISABDisturbanceCoords.disturbanceID";                

   mc2dbg2 << "[InfoSQL] query: '" << query_stream.str() << "'" << endl;
   if( !doQuery(sqlQuery.get(), query_stream.str().c_str(),
                "InfoSQL::getDisturbanceWithID()"))
   {
      mc2log << error << "InfoSQL::getDisturbanceWithID():"
             << " failed, returning false!" << endl;
      return false;
   }   
   
   uint32 prevDisturbanceID = MAX_UINT32;
   while( sqlQuery->nextRow() ) {
      //first found disturbanceid
      uint32 disturbanceID = uint32(atoi(sqlQuery->getColumn(0)));
      if( disturbanceID != prevDisturbanceID ) {
         //new disturbanceid...
         if( (foundDist != NULL) && (prevDisturbanceID != MAX_UINT32) ) {
            //... the second disturbanceid actually...
            if(foundDist->getNbrCoordinates() > 0 ) {
               //... and it has coordinates. Return it.
               return foundDist;
            }
         } 

         //this should be the first read line

         //extract the data
         MC2String situationReference = sqlQuery->getColumn(1);
         TrafficDataTypes::disturbanceType type =
            TrafficDataTypes::disturbanceType(atoi(sqlQuery->getColumn(2)));
         TrafficDataTypes::phrase phrase = 
            TrafficDataTypes::phrase(atoi(sqlQuery->getColumn(3)));
         uint32 eventCode    = uint32(atoi(sqlQuery->getColumn(4)));
         uint32 startTime    = uint32(atoi(sqlQuery->getColumn(5)));
         uint32 endTime      = uint32(atoi(sqlQuery->getColumn(6)));
         uint32 creationTime = uint32(atoi(sqlQuery->getColumn(7)));
         TrafficDataTypes::severity severity = 
            TrafficDataTypes::severity(atoi(sqlQuery->getColumn(8)));
         TrafficDataTypes::direction direction =
            TrafficDataTypes::direction(atoi(sqlQuery->getColumn(9)));
         MC2String text           = sqlQuery->getColumn(10);
         bool deleted       = bool(atoi(sqlQuery->getColumn(11)));
         MC2String firstLocation  = sqlQuery->getColumn(12);
         MC2String secondLocation = sqlQuery->getColumn(13);
         uint32 extent      = uint32(atoi(sqlQuery->getColumn(14)));
         uint32 costFactor  = uint32(atoi(sqlQuery->getColumn(15)));
         uint32 queueLength = uint32(atoi(sqlQuery->getColumn(16)));
         
         //create the DisturbanceElement
         foundDist =
            new DisturbanceElement(disturbanceID,
                                   situationReference,
                                   type,
                                   phrase,
                                   eventCode,
                                   startTime,
                                   endTime,
                                   creationTime,
                                   severity,
                                   direction,
                                   firstLocation,
                                   secondLocation,
                                   extent,
                                   costFactor,
                                   text,
                                   queueLength);
         foundDist->setDeleted(deleted);
      } 
      //read the coordinate for this line
      int32  lat        =  int32(atoi(sqlQuery->getColumn(19)));
      int32  lon        =  int32(atoi(sqlQuery->getColumn(20)));
      uint32 angle      = uint32(atoi(sqlQuery->getColumn(21)));
      uint32 routeIndex = uint32(atoi(sqlQuery->getColumn(22)));
      //and add to the disturbance.
      foundDist->addCoordinate(lat, lon, angle, routeIndex);
      
      prevDisturbanceID = disturbanceID;
   }
   return foundDist;
}

DisturbanceElement* 
InfoSQL::getDisturbance(const MC2String& situationReference)
{
   return getDisturbance(MAX_UINT32, situationReference);
}

DisturbanceElement*
InfoSQL::getDisturbance(uint32 distID)
{
   return getDisturbance(distID, "");
}


bool
InfoSQL::getTMCCoords(MC2String firstLocation,
                      MC2String secondLocation,
                      int32 extent,
                      TrafficDataTypes::direction direction,
                      vector< pair<int32, int32> > &firstCoords,
                      vector< pair<int32, int32> > &secondCoords)
{
   if (! checkDatabaseConnection() ){
      return false;
   }
   mc2dbg2 << "First location = " << firstLocation << endl;
   mc2dbg2 << "Second location = " << secondLocation << endl;
   char query[4096];
   CharEncSQLQuery* sqlQuery = NULL;
   
   if( secondLocation != "" ) {
      mc2dbg2 << "Second location != \"\"" << endl;
      sqlQuery = m_sqlConnection->newQuery();
      sprintf(query, "SELECT * FROM TMCLocation WHERE locationID = \"%s\"",
              firstLocation.c_str());
      if( !doQuery(sqlQuery, query, "IS: getTMCCoords()")) {
         mc2log << error << "IS: getTMCCoords failed" << endl;
         delete sqlQuery;
         return false;
      }
      while( sqlQuery->nextRow() ) {
         int32 latitude = int32(atoi(sqlQuery->getColumn(2)));
         int32 longitude = int32(atoi(sqlQuery->getColumn(3)));
         firstCoords.push_back(pair<int32, int32>( latitude, longitude));
      }
      delete sqlQuery;
      
      sqlQuery = m_sqlConnection->newQuery();
      sprintf(query, "SELECT * FROM TMCLocation WHERE locationID = \"%s\"",
              secondLocation.c_str());
      if( !doQuery(sqlQuery, query, "IS: getTMCCoords()")) {
         mc2log << error << "IS: getTMCCoords failed" << endl;
         delete sqlQuery;
         return false;
      }
      while( sqlQuery->nextRow() ) {
         int32 latitude = int32(atoi(sqlQuery->getColumn(2)));
         int32 longitude = int32(atoi(sqlQuery->getColumn(3)));
         secondCoords.push_back(pair<int32, int32>( latitude, longitude));
      }
      delete sqlQuery;
      if( firstCoords.size() > 0 && secondCoords.size() > 0 ) {
         return true;
      } else {
         return false;
      }
   } else {
      if( extent == 0 ) {
         sqlQuery = m_sqlConnection->newQuery();
         sprintf(query, "SELECT * FROM TMCLocation WHERE locationID = \"%s\"",
                 firstLocation.c_str());
         if( !doQuery(sqlQuery, query, "IS: getTMCCoords()")) {
            mc2log << error << "IS: getTMCCoords failed" << endl;
            delete sqlQuery;
            return false;
         }
         while( sqlQuery->nextRow() ) {
            int32 latitude = int32(atoi(sqlQuery->getColumn(2)));
            int32 longitude = int32(atoi(sqlQuery->getColumn(3)));
            firstCoords.push_back(pair<int32, int32>( latitude, longitude));
         }
         delete sqlQuery;
         if( firstCoords.size() > 0 ) {
            return true;
         } else {
            return false;
         }
      } else {
         secondLocation = firstLocation;
         uint32 nbrExtent = extent;
         if(extent < 0)
            nbrExtent = -extent;
         
         for( uint32 i = 0; i < nbrExtent; i++) {
            sqlQuery = m_sqlConnection->newQuery();
            if(extent < 0){
               sprintf(query, "SELECT negOffset FROM LocationTable2 "
                       "WHERE locationID = \"%s\"", secondLocation.c_str());
            } else {
               sprintf(query, "SELECT posOffset FROM LocationTable2 "
                       "WHERE locationID = \"%s\"", secondLocation.c_str());
            }
            if( !doQuery( sqlQuery, query, "IS::getTMCCoords()")) {
               mc2log << error << "IS::getTMCCoords(): failed!"
                      << endl;
               delete sqlQuery;
               return false;
            }
            if( sqlQuery->nextRow() ) {
               secondLocation = sqlQuery->getColumn(0);
            }
            if( (secondLocation == "E3300000") ||
                (secondLocation == "D0100000") )
            {
               delete sqlQuery;
               mc2dbg << "1)InfoSQL: No TMC-point found." << endl
                      << "secondLocation:" << secondLocation << endl;
               return false;
            }
            delete sqlQuery;
         }
         
         sqlQuery = m_sqlConnection->newQuery();
         sprintf(query, "SELECT * FROM TMCLocation WHERE locationID = \"%s\"",
                 firstLocation.c_str());
         if( !doQuery(sqlQuery, query, "IS: getTMCCoords()")) {
            mc2log << error << "IS: getTMCCoords failed" << endl;
            delete sqlQuery;
            return false;
         }
         while( sqlQuery->nextRow() ) {
            int32 latitude = int32(atoi(sqlQuery->getColumn(2)));
            int32 longitude = int32(atoi(sqlQuery->getColumn(3)));
            firstCoords.push_back(pair<int32, int32>( latitude, longitude));
         }
         delete sqlQuery;
      
         sqlQuery = m_sqlConnection->newQuery();
         sprintf(query, "SELECT * FROM TMCLocation WHERE locationID = \"%s\"",
                 secondLocation.c_str());
         if( !doQuery(sqlQuery, query, "IS: getTMCCoords()")) {
            mc2log << error << "IS: getTMCCoords failed" << endl;
            delete sqlQuery;
            return false;
         }
         while( sqlQuery->nextRow() ) {
            int32 latitude = int32(atoi(sqlQuery->getColumn(2)));
            int32 longitude = int32(atoi(sqlQuery->getColumn(3)));
            secondCoords.push_back(pair<int32, int32>( latitude, longitude));
         }
         delete sqlQuery;
         if( firstCoords.size() > 0 && secondCoords.size() > 0) {
            return true;
         } else {
            return false;
         }
      }              
   }
   return true;  
}

bool
InfoSQL::doQuery(SQLQuery* sqlQuery, 
                 const std::ostringstream& stream, 
                 const char* whereTag)
{
   return doQuery(sqlQuery, stream.str().c_str(), whereTag);
}


bool
InfoSQL::doQuery(SQLQuery* sqlQuery, const MC2String& str, 
                 const char* whereTag)
{
   return doQuery(sqlQuery, str.c_str(), whereTag);
}


bool
InfoSQL::doQuery(SQLQuery* sqlQuery, const char* query,
                 const char* whereTag)
{
   mc2dbg8 << "IS::doQuery(), query: " << query << ", tag: "
           << whereTag << endl;
   if( query == NULL ){
      return false;
   }
   mc2dbg4 << "Query: " << query << endl;
   if (! sqlQuery->prepare(query) ) {
      mc2log  << error << "IS::doQuery(): Problem preparing query at " 
              << whereTag << ": " << sqlQuery->getErrorString() << endl;
      mc2dbg4 << "Failed query: " << query << endl;
      return false;
   }
   
   if (! sqlQuery->execute() && sqlQuery->getError() > 0) {
      mc2log << error << "IS::doQuery(): Problem executing query at "
             << whereTag << ": " << sqlQuery->getErrorString() << endl;
      mc2log << error << "Failed query: " << query << endl;
      return false;
   }
   return true;
}

void
InfoSQL::addTable(const MC2String& name, const MC2String& createQuery,
                  const MC2String& extraQuery)
{
   m_tableData.push_back( TableData( name, createQuery, extraQuery ) );
}

size_t
InfoSQL::initTables()
{
   addTable("ISABDisturbance",
            "CREATE TABLE ISABDisturbance("
            "disturbanceID INT NOT NULL, "
            "situationReference VARCHAR(50) NOT NULL, "
            "type SMALLINT NOT NULL, phrase SMALLINT NOT NULL, "
            "eventCode INT NOT NULL, "
            "startTime INT NOT NULL, endTime INT NOT NULL, "
            "creationTime INT NOT NULL, "
            "severity SMALLINT NOT NULL, direction SMALLINT NOT NULL, "
            "text MEDIUMBLOB NOT NULL, "
            "deleted SMALLINT NOT NULL, firstLocation VARCHAR(10) NOT NULL, "
            "secondLocation VARCHAR(10) NOT NULL, extent INT NOT NULL, "
            "costFactor INT NOT NULL, queueLength INT NOT NULL, "
            "PRIMARY KEY(disturbanceID)) ");
   
   addTable("ISABDisturbanceCoords",
            "CREATE TABLE ISABDisturbanceCoords( "
            "disturbanceID INT NOT NULL, situationReference VARCHAR(50) "
            "NOT NULL, latitude INT NOT NULL, longitude INT NOT NULL, "
            "angle SMALLINT NOT NULL, "
            "routeIndex SMALLINT NOT NULL, "
            "PRIMARY KEY(disturbanceID, routeIndex))");
#if 0
   addTable("ISABUpdate",
            "CREATE TABLE ISABUpdate ( "
            "mapID INT NOT NULL, updateID INT NOT NULL, nodeID INT NOT NULL, "
            "fromNodeID INT NOT NULL, vehicles INT NOT NULL, "
            "lowNbr SMALLINT NOT NULL, highNbr SMALLINT NOT NULL, "
            "restrictions SMALLINT, speedLimit SMALLINT NOT NULL, "
            "validated SMALLINT, nameOffset SMALLINT , "
            "newName VARCHAR(255) NOT NULL,"
            "PRIMARY KEY (mapID))");
#endif
   addTable("LocationTable2",
            "CREATE TABLE LocationTable2( locationID VARCHAR(8) NOT NULL, "
            "roadNumber VARCHAR(50) NOT NULL, firstName VARCHAR(50) NOT NULL, "
            "secondName VARCHAR(50) NOT NULL, negOffset VARCHAR(8) NOT NULL, "
            "posOffset VARCHAR(50) NOT NULL, PRIMARY KEY(locationID) )");
   
   addTable("TMCLocation",
            "CREATE TABLE TMCLocation(locationID VARCHAR(10) NOT NULL, "
            "coordIndex INT NOT NULL, latitude INT NOT NULL, "
            "longitude INT NOT NULL, PRIMARY KEY(locationID, coordIndex) )");
   
   return m_tableData.size();
} 

bool
InfoSQL::initialCheckDatabase() {
   if (! checkDatabaseConnection() ){
      return false;
   }
   auto_ptr<CharEncSQLQuery> sqlQuery( m_sqlConnection->newQuery( ) );
   
   mc2dbg2 << "Doing initial database check" << endl;
   for( tableCont_t::const_iterator it = m_tableData.begin(); 
        it != m_tableData.end(); it++) {
      mc2dbg4 << "Checking table " << it->name << endl;
      if (! m_sqlConnection->tableExists(it->name)) {
         // create the table
         if (it->createQuery.empty()) {
            mc2log << warn << "Table " << it->name << " not found in "
                   << "the database and the table wasn't created since the "
                   << "create statement is empty!" << endl;
         } else {
            if (! sqlQuery->prepAndExec(it->createQuery) && 
                sqlQuery->getError() > 0) {
               mc2log << error << "Table " << it->name
                      << " not found in the database and "
                      << "table creation has failed with the error: " 
                      << sqlQuery->getErrorString() << endl;
               mc2log << error << "Table creation query: " 
                      << it->createQuery
                      << endl;
           
               return false;
            }
            if ( (! it->extraQuery.empty()) && 
                 (! sqlQuery->prepAndExec( it->extraQuery )) &&
                 ( sqlQuery->getError() > 0 ) ) {
               mc2log << error << " Extra SQL initialization for table"
                      << it->name << " has failed with the error: "
                      << sqlQuery->getErrorString() << endl;
               return false;
            }
            mc2dbg4 << "  <<>> created" << endl;
         }
      } else {
         mc2dbg4 << "  <<>> OK" << endl;
      }
   }

   return true;
}

uint32 
InfoSQL::getNewUniqueID(const char* tableName, const char* colName)
{
   if (! checkDatabaseConnection() ){ //verify that we can access the DB
      return 0;
   }
   std::ostringstream queryBaseStream;
   //Generate the constant part of the query
   queryBaseStream << "SELECT " << colName << " FROM " << tableName
                   << " WHERE " << colName << " = ";
   const MC2String queryBase = queryBaseStream.str(); //store in string
   //we need a query object
   auto_ptr<CharEncSQLQuery> sqlQuery( m_sqlConnection->newQuery() );
   while( true ){ //do until die
      //generate random value
      uint32 id = 1+(uint32)((float64)MAX_INT32*rand()/(RAND_MAX + 1.0));
      ostringstream queryStream;
      queryStream << queryBase << id;
      mc2dbg << ISQL << "id: " << id << " stream: " << queryStream.str() << endl;
      if( ! doQuery( sqlQuery.get(), queryStream, "ISQL:getNewUniqueID")){
         //The query failed, we better give up
         mc2log << error << ISQL << "getNewUniqueID() giving up!" << endl;
         return 0;
      } else if( ! sqlQuery->nextRow() ) {
         //the query returned no rows, use this id.
         return id;
      }
      //the id was already used, try another one. 
   }
}





