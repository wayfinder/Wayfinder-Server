/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExtradataExtractor.h"
#include "MySQLDriver.h"
#include "StringUtility.h"
#include "OldExtraDataUtility.h"
#include "StringTable.h"
#include "PropertyHelper.h"

#include <iostream>
#include <fstream>

ExtradataExtractor::ExtradataExtractor(const char* database,
                                       const char* host,
                                       const char* username,
                                       const char* password)
                                       
{
   mc2log << info << "ED database connection parameters:"  << endl;
   mc2log << info << "   Database: "  << database << endl;
   mc2log << info << "   Host: "      << host     << endl;
   mc2log << info << "   User name: " << username << endl;
   mc2log << info << "   Password: "  << password << endl;

   m_sqlConnection = new SQLConnection(new MySQLDriver(host, database,
                                                       username, password));
   m_subSqlConnection = new SQLConnection(new MySQLDriver(host, database,
                                                      username, password));
   
   mc2dbg1 << "ExtradataExtractor: connections created" << endl;
   m_connected = (m_sqlConnection->connect()) &&
                 (m_subSqlConnection->connect()); 
   if (!m_connected) {
      mc2log << fatal << "WASPExtractor: Not connected! "
             << " - perhaps something wrong with POI database settings" << endl;
      MC2_ASSERT(false);
   }
   mc2dbg1 << "ExtradataExtractor: connected" << endl;
}   

ExtradataExtractor::~ExtradataExtractor()
{
	   delete m_sqlConnection;
}

bool
ExtradataExtractor::doQuery(SQLQuery* sqlQuery, const char* query,
                                   const char* whereTag)
{   
   mc2dbg8 << "ExtradataExtractor::doQuery(), query: " << query
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
ExtradataExtractor::extractExtradata(char* insertType, char* country,
                                     uint32 countryID,
                                     char* version, const char* insertDate)
{

   if (!m_connected) {
      mc2log << fatal << "Not connected" << endl;
      return false;
   }
   
   //mc2log << "insertType=" << insertType << " country=" << country 
   mc2log << "insertType=" << insertType << " countryID=" << countryID
          << " country=" << country
          << " version=" << version << endl;
   
   SQLQuery* sqlQuery = m_sqlConnection->newQuery();
   if ( countryID == MAX_UINT32 ) {
      //Select ID for given country 
      char countryIDQuery[2000];
      sprintf(countryIDQuery,
      "SELECT ID FROM POICountries "
      "WHERE country LIKE %s%s%s;", "\"", country, "\"");
      if ( ! doQuery(sqlQuery, countryIDQuery, 
                     "ExtradataExtractor::extractExtradata") ) {
         delete sqlQuery;
         return false;
      }
      while (sqlQuery->nextRow()) {
         countryID = atoi(sqlQuery->getColumn(0));
      }
   }
   
   //Select ID for given version
   char versionIDQuery[2000];
   sprintf(versionIDQuery,
   "SELECT ID FROM EDVersion "
   "WHERE version LIKE %s%s%s;", "\"", version, "\"");
   if ( ! doQuery(sqlQuery, versionIDQuery, 
                  "ExtradataExtractor::extractExtradata") ) {
      delete sqlQuery;
      return false;
   }
   uint32 versionID = MAX_UINT32;
   while (sqlQuery->nextRow()) {
      versionID = atoi(sqlQuery->getColumn(0));
   }
   
   //Select ID from given insertType
   uint32 insertTypeID = MAX_UINT32;
   char andInsertTypeQuery[2000];
   if (StringUtility::strcasecmp(insertType, "allInsertTypes") == 0) {
      insertTypeID = 0;
      sprintf(andInsertTypeQuery, " insertType > %i", insertTypeID);
   } else {
      char insertTypeIDQuery[2000]; 
      sprintf(insertTypeIDQuery,
         "SELECT ID FROM EDInsertTypes "
         "WHERE insertTypeName LIKE %s%s%s;", "\"",insertType,"\"");
      if ( ! doQuery(sqlQuery, insertTypeIDQuery, 
                  "ExtradataExtractor::extractExtradata") ) {
         delete sqlQuery;
         return false;
      }
      while (sqlQuery->nextRow()) {
         insertTypeID = atoi(sqlQuery->getColumn(0));
         sprintf(andInsertTypeQuery, " insertType = %i", insertTypeID);
      }
   }
   
   // Check if given country, version or insert type  is valid
   mc2log << "insertTypeID=" << insertTypeID << " countryID=" << countryID 
          << " versionID=" << versionID << " date=\"" << insertDate << "\"" 
          << endl; 
   if ( (countryID == MAX_UINT32) || (versionID == MAX_UINT32) || 
         (insertTypeID == MAX_UINT32)){
      mc2log << fatal << "Invalid country, version or insertType";
      if (countryID != MAX_UINT32)
         mc2log << "; country ok";
      if (versionID != MAX_UINT32)
         mc2log << "; version ok";
      if (insertTypeID != MAX_UINT32)
         mc2log << "; insertType ok";
      mc2log << endl;
      MC2_ASSERT(false);
   }

   // Check that date-string is given and/or valid.
   bool dateStringGiven = false;
   bool insertDateOK = false;
   if ((StringUtility::strcasecmp(insertDate, "1900-00-00 00:00:00") == 0)) {
      insertDateOK = true;
   } else {
      dateStringGiven = true;
      // Date given
      if ( (strlen(insertDate) == 10) &&
           (insertDate[4] == '-') && (insertDate[7] == '-') ) {
         insertDateOK = true;
      }
      // Date and time given
      if ( (strlen(insertDate) == 19) &&
           (insertDate[4] == '-') && (insertDate[7] == '-') &&
           (insertDate[10] == ' ') && (insertDate[13] == ':') &&
           (insertDate[16] == ':') ) {
         insertDateOK = true;
      }
   }
   if (!insertDateOK) {
      mc2log << fatal << "Invalid insert date string \"" << insertDate 
             << "\"" << endl;
      return false;
   }


   // FIXME: only want to extract map corrections that are created for the
   // map supplier of this map. E.g. if the version (o_selectVersion) is
   // OpenStreetMap_201005, only extract extradata records that have
   // validFrom NULL or validFrom any OpenStreetMap version.
   
   //Extract extradatarecords within given criterias 
   char EDquery[2000];
   sprintf(EDquery, 
           "SELECT EDMain.ID, source, writer, added, TAreference, edRecord, "
           "EDInsertTypes.insertTypeName, comment, validFromVersion, "
           "orgValue, groupID, mapID "
           "FROM EDMain, EDInsertTypes "
           "WHERE EDMain.insertType=EDInsertTypes.ID "
           "AND added >= %s%s%s "
           "AND active = 1 "
           "AND %s "
           "AND (validFromVersion <= %i "
           "OR validFromVersion IS NULL) "
           "AND (validToVersion >= %i "
           "OR validToVersion IS NULL) "
           "AND EDMain.country = %i;",
           "\"",insertDate,"\"", andInsertTypeQuery, 
           versionID, versionID, countryID);
   
   if ( ! doQuery(sqlQuery, EDquery, "ExtradataExtractor::extractExtradata") ) {
      delete sqlQuery;
      return false;
   }
   
   // sub query for version if comment should be printed
   SQLQuery* subSqlQuery = m_subSqlConnection->newQuery();
   
   //Print extradatarecords
   uint32 nbrRecords = 0;
   while (sqlQuery->nextRow()) {
      const char*  ID= sqlQuery->getColumn(0);
      const char*  source= sqlQuery->getColumn(1);
      const char*  writer= sqlQuery->getColumn(2);
      const char*  added= sqlQuery->getColumn(3);
      const char*  TAreference= sqlQuery->getColumn(4);
      const char*  edRecord= sqlQuery->getColumn(5);
      const char*  insertTypeName= sqlQuery->getColumn(6);
      const char*  comment= sqlQuery->getColumn(7);
      const char*  validFromVersion= sqlQuery->getColumn(8);
      const char*  orgValue= sqlQuery->getColumn(9);
      const char*  groupID= sqlQuery->getColumn(10);
      const char*  mapID= sqlQuery->getColumn(11);

      
      // Print comment if date is given (= for dynamic extra data)
      if ( dateStringGiven ) {
         //Get version name (for which the record was created)
         const char* versionName = "";
         if (StringUtility::strcasecmp(validFromVersion, "") != 0) {
            char versionQuery[2000];
            sprintf(versionQuery,
                    "SELECT version from EDVersion where ID=%s;",
                    validFromVersion);
        
            if ( ! doQuery(subSqlQuery, versionQuery,
                           "Extradata::extractExtradata") ) {
               delete subSqlQuery;
               return false;
            }
            subSqlQuery->nextRow();
            versionName = subSqlQuery->getColumn(0);
         }
         
         vector<MC2String> logComments;
         MC2String logAdded = "# ";
         logAdded.append(added);
         logComments.push_back( logAdded );
         logComments.push_back( writer );
         logComments.push_back( source );
         logComments.push_back( comment );
         logComments.push_back( orgValue );
         logComments.push_back( TAreference );
         logComments.push_back( ID );
         logComments.push_back( insertTypeName );
         logComments.push_back( versionName );
         logComments.push_back( groupID );
         logComments.push_back( country );
         logComments.push_back( mapID );
         MC2String logComment = 
            OldExtraDataUtility::createEDFileString( logComments, true );
         cout << logComment << endl;

      }

      // Print extra data record
      cout << edRecord << endl;
      cout << endl;
      nbrRecords++;
   
   }
   mc2log << "Extracted " << nbrRecords << " records for country "
          << StringTable::getString(
               StringTable::getCountryStringCode( 
                  StringTable::countryCode(countryID) ),
               StringTable::ENGLISH) << endl;
   
   //Check that no errors occured in sqlQuery
   if (sqlQuery->getError() > 0) {
      mc2log << error << here << "Problem while executing query:" 
             << sqlQuery->getErrorString() << endl;
      return false;
   }
                                     
   delete sqlQuery;
   return true;
}
      
   

int main(int argc, char* argv[])
{
   MC2String mydb = PropertyHelper::get<MC2String>("POI_SQL_DATABASE").c_str();
   MC2String myhost = PropertyHelper::get<MC2String>("POI_SQL_HOST").c_str();
   MC2String myuser = PropertyHelper::get<MC2String>("POI_SQL_USER").c_str();
   MC2String mypwd = PropertyHelper::get<MC2String>("POI_SQL_PASSWORD").c_str();
   const char* database = mydb.c_str();
   const char* host = myhost.c_str();
   const char* username = myuser.c_str();
   const char* password = mypwd.c_str();

   CommandlineOptionHandler coh(argc, argv,0);
   char summaryStr[300];
   sprintf(summaryStr,
           "Use for extracting map correction records (extradata records) "
           "from db in map generation.\n"
           "Connects to db: %s on: %s.",
           database, host );
   coh.setSummary( summaryStr );
   //coh.setTailHelp("countryID");
   

   //-------------------------------------Add extradata to Map?
   char* o_extractExtradata = NULL;
   coh.addOption("-t", "--extractExtradata",
		           CommandlineOptionHandler::stringVal,
                 1, &o_extractExtradata, "",
                 "Extracts all extradata from WASP "
                 "for a given country, mapversion and inserttype. "
                 "Give the wanted inserttype for this option, possible "
                 "types are:\n"
                 "\tbeforeInternalConnections\n"
                 "\tbeforeGenerateStreets\n"
                 "\tbeforeGenerateTurndescriptions\n"
                 "\tafterGenerateTurndescriptions\n"
                 "\tallInsertTypes\n"
                 "This option must be used together with "
                 "option c and r");
       
   //--------------------------------------Select country
   char* o_selectCountry = NULL;
   coh.addOption("-c", "--selectCountry",
                 CommandlineOptionHandler::stringVal,
                 1, &o_selectCountry, "",
                 "Select for which country the extradata should be extracted\n"
                 "England\n"
                 "Sweden\n"
                 "Germany\n"
                 "Denmark\n"
                 "etc..\n"
                 "Check in WASP POICountries table for available countries and "
                 "spelling. Countries made up by several words can be given "
                 "either as \"czech republic\" or \"czech_republic\"\n."
                 );

   //--------------------------------------Select country ID
   uint32 o_selectCountryID = MAX_UINT32;
   coh.addOption("-i", "--selectCountryID",
                 CommandlineOptionHandler::uint32Val,
                 //1, &o_selectCountryID, strmaxuint32.c_str(),
                 1, &o_selectCountryID, "",
                 "ID of the country for which extradata should be extracted.\n"
                 "Check in WASP POICountries table for available country IDs.\n"
                 "If country ID is not given, the --selectCountry country "
                 "name option is used, to lookup country ID in WASP "
                 "POICountries."
                 );

   //---------------------------------------Select version
   char* o_selectVersion = NULL;
   coh.addOption("-r", "--selectVersion",
         
                 CommandlineOptionHandler::stringVal,
                 1, &o_selectVersion, "",
                 "Select the version of the maps to be generated "
                 "for example TeleAtlas_2010_06 or OpenStreetMap_201005.");
   
   //---------------------------------------Add only latest extradata 
   //(after merge) 
   const char* o_selectInsertDate = NULL;
   coh.addOption("-d", "--selectInsertDate",

                 CommandlineOptionHandler::stringVal,
                 1, &o_selectInsertDate, "",
                 "With this option only extradata inserted into WASP after "
                 "a given date will be extracted, for example:\n"
                 "\"2010-05-25 09:00:00\" (don't forget \"-chars) "
                 "or simply 2010-05-25 (implies from 00:00:00). "
                 "The purpose of this action is to extract newly created "
                 "extradata for dynamic map generation.");
   
   // Parse command-line
   if(!coh.parse()) {
      cerr << argv[0] << ": Error on commandline! (-h for help)" << endl;
      exit(1);
   }           
   ExtradataExtractor* ext = new ExtradataExtractor(database, host,
                                                    username, password);
   
   if ( (!o_selectInsertDate) ) {
      o_selectInsertDate = "1900-00-00 00:00:00";
   }
   
   //Extract extradata
   if ( (o_extractExtradata) ) {
      bool result = ext->extractExtradata(o_extractExtradata, 
                                          o_selectCountry,
                                          o_selectCountryID,
                                          o_selectVersion, 
                                          o_selectInsertDate);
      if ( !result ){
         mc2log << error << "Failed to extract extra data" << endl;
         exit(1);
      }
   }

   delete ext;
}

