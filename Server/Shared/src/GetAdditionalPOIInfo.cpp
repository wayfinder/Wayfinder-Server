/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <memory>

#include "GetAdditionalPOIInfo.h"
#include "MySQLDriver.h"
#include "StringUtility.h"
#include "STLStringUtility.h"
#include "Properties.h"
#include "NameUtility.h"
#include "CharEncSQLConn.h"
#include "CharEncSQLQuery.h"
#include "GenericMap.h"
#include "PointOfInterestItem.h"
#include "POIInfo.h"
#include "StringConvert.h"
#include "ItemInfoEntry.h"
#include "ItemInfoFilter.h"
#include <fstream>

namespace {
/**
 *    Get a combined "min / max" string using the given
 *    min and max strings. If any of the strings are empty
 *    use "-".
 */
MC2String
getMinMaxString( MC2String min, MC2String max)
{
   // Ceate string "min / max"
   MC2String infoString = "";
   
   if ( min[ 0 ] != '\0' ) {
      infoString += min;
   } else  {
      infoString = "-";
   }
   infoString += " / ";
   if ( max[ 0 ] != '\0' ) {
      infoString += max;
   } else  {
      infoString += "-";
   }

   return infoString;
}
}

// Table that used to be here has been moved to ItemInfoEnums
// so that it works for ItemInfoEnums, not only database keys.

ItemInfoEnums::InfoType GetAdditionalPOIInfo::m_keyID_to_type[] = 
{
   ItemInfoEnums::vis_address,        // |  0 | Vis. address             |
   ItemInfoEnums::vis_house_nbr,      // |  1 | Vis. house nbr           |
   ItemInfoEnums::vis_zip_code,       // |  2 | Vis. zip code            |
   ItemInfoEnums::vis_complete_zip,   // |  3 | Vis. complete zip        |
   ItemInfoEnums::phone_number,       // |  4 | Phone                    |
   ItemInfoEnums::Vis_zip_area,       // |  5 | Vis. zip area            |
   ItemInfoEnums::vis_full_address,   // |  6 | Vis. full address        |
   ItemInfoEnums::fax_number,         // |  7 | Fax                      |
   ItemInfoEnums::email,              // |  8 | Email                    |
   ItemInfoEnums::url,                // |  9 | URL                      |
   ItemInfoEnums::brandname,          // | 10 | Brandname                |
   ItemInfoEnums::short_description,  // | 11 | Short description        |
   ItemInfoEnums::long_description,   // | 12 | Long description         |
   ItemInfoEnums::citypart,           // | 13 | Citypart                 |
   ItemInfoEnums::state,              // | 14 | State                    |
   ItemInfoEnums::neighborhood,       // | 15 | Neighborhood             |
   ItemInfoEnums::open_hours,         // | 16 | Open hours               |
   ItemInfoEnums::nearest_train,      // | 17 | Nearest train            |
   ItemInfoEnums::start_date,         // | 18 | Start date               |
   ItemInfoEnums::end_date,           // | 19 | End date                 |
   ItemInfoEnums::start_time,         // | 20 | Start time               |
   ItemInfoEnums::end_time,           // | 21 | End time                 |
   ItemInfoEnums::accommodation_type, // | 22 | Accommodation type       |
   ItemInfoEnums::check_in,           // | 23 | Check in                 |
   ItemInfoEnums::check_out,          // | 24 | Check out                |
   ItemInfoEnums::nbr_of_rooms,       // | 25 | Nbr of rooms             |
   ItemInfoEnums::single_room_from,   // | 26 | Single room from         |
   ItemInfoEnums::double_room_from,   // | 27 | Double room from         |
   ItemInfoEnums::triple_room_from,   // | 28 | Triple room from         |
   ItemInfoEnums::suite_from,         // | 29 | Suite from               |
   ItemInfoEnums::extra_bed_from,     // | 30 | Extra bed from           |
   ItemInfoEnums::weekend_rate,       // | 31 | Weekend rate             |
   ItemInfoEnums::nonhotel_cost,      // | 32 | Nonhotel cost            |
   ItemInfoEnums::breakfast,          // | 33 | Breakfast                |
   ItemInfoEnums::hotel_services,     // | 34 | Hotel services           |
   ItemInfoEnums::credit_card,        // | 35 | Credit card              |
   ItemInfoEnums::special_feature,    // | 36 | Special feature          |
   ItemInfoEnums::conferences,        // | 37 | Conferences              |
   ItemInfoEnums::average_cost,       // | 38 | Average cost             |
   ItemInfoEnums::booking_advisable,  // | 39 | Booking advisable        |
   ItemInfoEnums::admission_charge,   // | 40 | Admission charge         |
   ItemInfoEnums::home_delivery,      // | 41 | Home delivery            |
   ItemInfoEnums::disabled_access,    // | 42 | Disabled access          |
   ItemInfoEnums::takeaway_available, // | 43 | Takeaway available       |
   ItemInfoEnums::allowed_to_bring_alcohol,// |44|Allowed to bring alcohol|
   ItemInfoEnums::type_food,          // | 45 | Type food                |
   ItemInfoEnums::decor,              // | 46 | Decor                    |
   ItemInfoEnums::text,               // | 47 | Display class            |
   ItemInfoEnums::image_url,          // | 48 | Image                    |
   ItemInfoEnums::supplier,           // | 49 | Supplier                 |
   ItemInfoEnums::owner,              // | 50 | Owner                    |
   ItemInfoEnums::price_petrol_superplus,// |51| Price petrol superplus  |
   ItemInfoEnums::price_petrol_super, // | 52 | Price petrol super       |
   ItemInfoEnums::price_petrol_normal,// | 53 | Price petrol normal      |
   ItemInfoEnums::price_diesel,       // | 54 | Price diesel             |
   ItemInfoEnums::price_biodiesel,    // | 55 | Price bio diesel         |
   ItemInfoEnums::free_of_charge,     // | 56 | Free of charge           |
   ItemInfoEnums::dont_show,          // | 57 | POIMain.rights           |
   ItemInfoEnums::dont_show,          // | 58 | myChild                  |
   ItemInfoEnums::dont_show,          // | 59 | myParent                 |
   ItemInfoEnums::open_for_season,    // | 60 | Open season              |
   ItemInfoEnums::dont_show,          // | 61 | Ski mountain max height  |
   ItemInfoEnums::dont_show,          // | 62 | Ski mountain min height  |
   ItemInfoEnums::dont_show,          // | 63 | Snow depth mountain      |
   ItemInfoEnums::dont_show,          // | 64 | Snow depth valley        |
   ItemInfoEnums::snow_quality,       // | 65 | Snow quality             |
   ItemInfoEnums::dont_show,          // | 66 | Lifts total              |
   ItemInfoEnums::dont_show,          // | 67 | Lifts open               |
   ItemInfoEnums::dont_show,          // | 68 | Slopes total km          |
   ItemInfoEnums::dont_show,          // | 69 | Slopes open km           |
   ItemInfoEnums::dont_show,          // | 70 | Valley run               |
   ItemInfoEnums::dont_show,          // | 71 | Valley run info          |
   ItemInfoEnums::dont_show,          // | 72 | Cross country skiing     |
   ItemInfoEnums::cross_country_skiing_km, // |73|Cross country skiing tot km|
   ItemInfoEnums::dont_show,          // | 74 | Funpark                  |
   ItemInfoEnums::dont_show,          // | 75 | Funpark info             |
   ItemInfoEnums::dont_show,          // | 76 | Night skiing             |
   ItemInfoEnums::dont_show,          // | 77 | Night skiing info        |
   ItemInfoEnums::glacier_area,       // | 78 | Glacier area             |
   ItemInfoEnums::last_snowfall,      // | 79 | Last snowfall            |
   ItemInfoEnums::dont_show,          // | 80 | Halfpipe                 |
   ItemInfoEnums::dont_show,          // | 81 | Halfpipe info            |
   ItemInfoEnums::ski_mountain_min_max_height, // |82|Mountain height (min/max m)|
   ItemInfoEnums::snow_depth_valley_mountain, // |83|Snow depth (valley/mountain cm) |
   ItemInfoEnums::lifts_open_total,   // | 84 | Lifts (open/total)         |
   ItemInfoEnums::ski_slopes_open_total, // | 85 | Slopes (open/total km)|
   ItemInfoEnums::dont_show,          // | 86 | parking company phone number |
   ItemInfoEnums::static_id,          // | 87 | Not really in this type
   ItemInfoEnums::dont_show,          // | 88 | Major road feature
   ItemInfoEnums::has_service,        // | 89 | Has service
   ItemInfoEnums::dont_show,          // | 90 | Has fuel Super95
   ItemInfoEnums::dont_show,          // | 91 | Has fuel Super98
   ItemInfoEnums::dont_show,          // | 92 | Has fuel Normal91
   ItemInfoEnums::dont_show,          // | 93 | Has fuel PKWDiesel
   ItemInfoEnums::dont_show,          // | 94 | Has fuel BioDiesel
   ItemInfoEnums::dont_show,          // | 95 | Has fuel NatGas
   ItemInfoEnums::has_carwash,        // | 96 | Has carwash
   ItemInfoEnums::has_24h_self_service_zone, // | 97 | Has 24h self service zone
   ItemInfoEnums::drive_in,           // | 98 | Drive In
   ItemInfoEnums::dont_show,          // | 99 | Unique icon
   ItemInfoEnums::dont_show,          // |100 | Sub type icon
   ItemInfoEnums::mailbox_collection_time, // |101 | Mailbox collection times
   ItemInfoEnums::dont_show,          // | 102 | Performer/Group/Band/Artist
   ItemInfoEnums::booking_url,        // | 103 | Booking URL
   ItemInfoEnums::booking_phone_number,//| 104 | Booking phone number
   ItemInfoEnums::dont_show,          // | 105 | POI url
   ItemInfoEnums::dont_show,          // | 106 | POI thumb
   ItemInfoEnums::dont_show,          // | 107 | average_rating
   ItemInfoEnums::dont_show,          // | 108 | provider_info
};


int GetAdditionalPOIInfo::m_keyID_value_is_bool[] = 
{
   0,               // |  0 | Vis. address             |
   0,               // |  1 | Vis. house nbr           |
   0,               // |  2 | Vis. zip code            |
   0,               // |  3 | Vis. complete zip        |
   0,               // |  4 | Phone                    |
   0,               // |  5 | Vis. zip area            |
   0,               // |  6 | Vis. full address        |
   0,               // |  7 | Fax                      |
   0,               // |  8 | Email                    |
   0,               // |  9 | URL                      |
   0,               // | 10 | Brandname                |
   0,               // | 11 | Short description        |
   0,               // | 12 | Long description         |
   0,               // | 13 | Citypart                 |
   0,               // | 14 | State                    |
   0,               // | 15 | Neighborhood             |
   0,               // | 16 | Open hours               |
   0,               // | 17 | Nearest train            |
   0,               // | 18 | Start date               |
   0,               // | 19 | End date                 |
   0,               // | 20 | Start time               |
   0,               // | 21 | End time                 |
   0,               // | 22 | Accommodation type       |
   0,               // | 23 | Check in                 |
   0,               // | 24 | Check out                |
   0,               // | 25 | Nbr of rooms             |
   0,               // | 26 | Single room from         |
   0,               // | 27 | Double room from         |
   0,               // | 28 | Triple room from         |
   0,               // | 29 | Suite from               |
   0,               // | 30 | Extra bed from           |
   1,               // | 31 | Weekend rate             |
   0,               // | 32 | Nonhotel cost            |
   1,               // | 33 | Breakfast                |
   0,               // | 34 | Hotel services           |
   0,               // | 35 | Credit card              |
   0,               // | 36 | Special feature          |
   0,               // | 37 | Conferences              |
   0,               // | 38 | Average cost             |
   1,               // | 39 | Booking advisable        |
   0,               // | 40 | Admission charge         |
   1,               // | 41 | Home delivery            |
   1,               // | 42 | Disabled access          |
   1,               // | 43 | Takeaway available       |
   1,               // | 44 | Allowed to bring alcohol |
   0,               // | 45 | Type food                |
   0,               // | 46 | Decor                    |
   0,               // | 47 | Display class            |
   0,               // | 48 | Image                    |
   0,               // | 49 | Supplier                 |
   0,               // | 50 | Owner                    |
   0,               // | 51 | Price petrol superplus   |
   0,               // | 52 | Price petrol super       |
   0,               // | 53 | Price petrol normal      |
   0,               // | 54 | Price diesel             |
   0,               // | 55 | Price bio diesel         |
   1,               // | 56 | Free of charge           |
   0,               // | 57 | POIMain.rights                |
   0,               // | 58 | myChild                       |
   0,               // | 59 | myParent                      |
   1,               // | 60 | Open season                   |
   0,               // | 61 | Ski mountain max height       |
   0,               // | 62 | Ski mountain min height       |
   0,               // | 63 | Snow depth mountain           |
   0,               // | 64 | Snow depth valley             |
   0,               // | 65 | Snow quality                  |
   0,               // | 66 | Lifts total                   |
   0,               // | 67 | Lifts open                    |
   0,               // | 68 | Slopes total km               |
   0,               // | 69 | Slopes open km                |
   1,               // | 70 | Valley run                    |
   0,               // | 71 | Valley run info               |
   1,               // | 72 | Cross country skiing          |
   0,               // | 73 | Cross country skiing total km |
   1,               // | 74 | Funpark                       |
   0,               // | 75 | Funpark info                  |
   1,               // | 76 | Night skiing                  |
   0,               // | 77 | Night skiing info             |
   1,               // | 78 | Glacier area                  |
   0,               // | 79 | Last snowfall                 |
   1,               // | 80 | Halfpipe                      |
   0,               // | 81 | Halfpipe info                 |
   0,               // | 82 | Mountain height (min/max m)   |
   0,               // | 83 | Snow depth (valley/mountain cm) |
   0,               // | 84 | Lifts (open/total)            |
   0,               // | 85 | Slopes (open/total km)        |
   0,               // | 86 | parking company phone number  |
   0,               // | 87 | used for static id Not really in this type
   0,               // | 88 | Major road feature
   1,               // | 89 | Has service
   1,               // | 90 | Has fuel Super95
   1,               // | 91 | Has fuel Super98
   1,               // | 92 | Has fuel Normal91
   1,               // | 93 | Has fuel PKWDiesel
   1,               // | 94 | Has fuel BioDiesel
   1,               // | 95 | Has fuel NatGas
   1,               // | 96 | Has carwash
   1,               // | 97 | Has 24h self service zone
   1,               // | 98 | Drive In
   0,               // | 99 | Unique icon
   0,               // |100 | Sub type icon
   0,               // |101 | Mailbox collection times
   0,               // |102 | Performer/Band/Artist
   0,               // | 103 | Booking URL
   0,               // | 104 | Booking phone number
   0,               // | 105 | POI URL
   0,               // | 106 | POI thumb
   0,               // | 107 | Average rating
   0,               // | 108 | Provider Info
};

uint32 GetAdditionalPOIInfo::getNbrKeys() {
   return sizeof( m_keyID_to_type ) / sizeof( m_keyID_to_type[0] );
}

GetAdditionalPOIInfo::GetAdditionalPOIInfo(const char* database,
                                           const char* host,
                                           const char* username,
                                           const char* password)
{

   m_database = StringUtility::newStrDup(database ? database : "");
   m_host     = StringUtility::newStrDup(host     ? host     : "");
   m_username = StringUtility::newStrDup(username ? username : "");
   m_password = StringUtility::newStrDup(password ? password : "");

   m_sqlConnection = NULL;
   m_waitDBRetry = 0;
  
   if (checkDatabaseConnection(3)) {
      mc2dbg1 << "GetAdditionalPOIInfo: connections created" << endl;
   } else {
      mc2log << error << "GetAdditionalPOIInfo::GetAdditionalPOIInfo Failed "
             << "to connect to database!" << " database = "
             << MC2CITE(m_database) << ", host = " << MC2CITE(m_host)
             << ", username = " << MC2CITE(m_username) << endl;
   }
}

GetAdditionalPOIInfo::~GetAdditionalPOIInfo()
{
   delete m_sqlConnection;
   delete [] m_database;
   delete [] m_host;
   delete [] m_username;
   delete [] m_password;
}

bool
GetAdditionalPOIInfo::checkDatabaseConnection(uint32 maxNbrTries)
{
   if (m_waitDBRetry > 0) {
      m_waitDBRetry--;
      return false;
   }

   if ((m_sqlConnection != NULL) && (m_sqlConnection->ping())) {
      return true;
   }

   mc2log << warn << "GetAdditionalPOIInfo::checkDatabaseConnection "
          << "No connection to database, trying to connect..." << endl;

   // Get POI database char encoding from prop file.
   const char* tmpSqlChEnc  = Properties::getProperty("POI_SQL_CHARENCODING");
   MC2String poiDBCharEncStr = "ISO-8859-1";
   if ( tmpSqlChEnc != NULL ){
      mc2dbg << "POI type in mc2.prop: " << tmpSqlChEnc << endl;
      poiDBCharEncStr = tmpSqlChEnc;
   }
   else {
      mc2dbg << "POI type in mc2.prop: NULL"  << endl;  
   }

   CharEncodingType::charEncodingType poiDBCharEnc = 
      CharEncoding::encStringToEncType( poiDBCharEncStr );
   CharEncodingType::charEncodingType mc2CharEnc = 
      CharEncoding::getMC2CharEncoding();

   mc2log << info << "Character encodings in GetAdditionalPOIInfo:" 
          << endl;
   mc2log << info << "   POI database: "
          << CharEncoding::encTypeToEncString (poiDBCharEnc) << endl;
   mc2log << info << "   Server code: " 
          << CharEncoding::encTypeToEncString (mc2CharEnc) << endl;



   uint32 i = 0;
   while (i < maxNbrTries) {
      delete m_sqlConnection;
      m_sqlConnection = NULL;
      m_sqlConnection = 
         new CharEncSQLConn( new MySQLDriver( m_host, 
                                              m_database, 
                                              m_username, 
                                              m_password),
                             poiDBCharEnc,
                             mc2CharEnc );

      if (m_sqlConnection->connect())
         return true;
      ++i;
   }
   m_waitDBRetry = 25;
   return false;
}

int
GetAdditionalPOIInfo::
getMultiInfo( const set<uint32>& waspIDs,
              const set<GetAdditionalPOIInfo::keyID_t>& wantedTypes,
              poiResultMap_t& result )
{
   if ( !checkDatabaseConnection() ) {
      return -1;
   }

   // Temporary vector
   vector<uint32> waspVect( waspIDs.begin(), waspIDs.end() );
   const uint32* begin = &waspVect.front();
   const uint32* end = begin + waspVect.size();
   return precache( begin, end,
                    wantedTypes, result );
}
namespace {
/** 
 * Adds a range of ids to a query string within '(' and ')'.
 * @param query the string to add ids to
 * @param begin start of id range
 * @param end end of id range
 */
void addIDs( MC2String& query, 
             const uint32* begin, const uint32* end ) {
   query += "(";
   for ( const uint32* it = begin; it != end; ++it ) {
      if ( it != begin ) {
         query += ", ";
      }
      char tmp[12];
      sprintf( tmp, "%u", (unsigned int)*it);
      query += tmp;
   }
   query += ")";
}

}

int GetAdditionalPOIInfo::getDynamicIDs( set<uint32>& ids ) { 
   MC2String query( "SELECT distinct poiID FROM POIInfoDynamic" );
   
   auto_ptr<CharEncSQLQuery> sqlQuery ( m_sqlConnection->newQuery() );

   if ( ! doQuery( sqlQuery.get(), query.c_str(),
                   "GetAdditionalPOIInfo::getDynamicIDs" ) ) {
      return -1;
   }

   while ( sqlQuery->nextRow() ) try {

      ids.insert( StringConvert::convert<uint32>( sqlQuery->getColumn( 0 ) ) );

   } catch ( const StringConvert::ConvertException& e ) {
      mc2log << warn << "[GetAdditionalPOIInfo::getDynamicIDs] will not add"
             << " dynamic information for id: " << sqlQuery->getColumn( 0 ) 
             << ". reason: " << e.what() << endl;
   }

   return 0;
}

int GetAdditionalPOIInfo::addDynamicInfo( uint32 dbID,
                                          POIInfo& info ) { 
   char buff[256];
   snprintf( buff, 256,
             "SELECT keyID,lang,val FROM POIInfoDynamic where poiID = %u", 
             dbID );

   auto_ptr<CharEncSQLQuery> sqlQuery ( m_sqlConnection->newQuery() );

   if ( ! doQuery( sqlQuery.get(), buff,
                   "GetAdditionalPOIInfo::addDynamicInfo" ) ) {
      return -1;
   }
   
   using namespace StringConvert;

   while ( sqlQuery->nextRow() ) try {
      
      uint32 keyID = convert<uint32>( sqlQuery->getColumn( 0 ) );
      LangTypes::language_t lang = static_cast<LangTypes::language_t>
         ( convert<uint32> ( sqlQuery->getColumn( 1 ) ) );

      info.addInfo( new POIInfoData( keyID, lang, sqlQuery->getColumn( 2 ) ) );

   } catch ( const ConvertException& e ) {
      mc2log << warn << "[GetAdditionalPOIInfo::addDynamicInfo] "
             << "will not add info for query: " << buff
             << ". Reason: " << e.what() << endl;
   }

   return 0;
}

int GetAdditionalPOIInfo::getStaticIDs( const uint32* beginIDs,
                                        const uint32* endIDs,
                                        vector<uint32>& ids ) try {

   //
   // The static IDs are requested in the following way:
   //  
   // 1) if number of ids are too many then split request in to two
   //
   // 2) Create a request string of all the poi IDs
   // 
   // 3) map each poi ID to an offset in the ids vector so
   //    the order of poi IDs are kept from begin -> end in the 
   //    ids out-vector.
   //

   const uint32 numberOfIDs = endIDs - beginIDs;
   // nothing to do with zero ids
   if ( numberOfIDs == 0 ) {
      return 0;
   }

   // if number exceeds the maximum request then 
   // split the request into two
   if ( numberOfIDs > Properties::getUint32Property( "POI_SQL_MAX_PRECACHE",
                                                     10000 ) ) {
      const uint32 *middle = beginIDs + numberOfIDs / 2;
      return getStaticIDs( beginIDs, middle, ids ) + 
         getStaticIDs( middle, endIDs, ids );
   }


   map<uint32, uint32> idMap;
   {
      // build offset map
      uint32 i = 0;
      for ( const uint32* it = beginIDs; it != endIDs; ++it, ++i ) {
         idMap[ *it ] = i;
      }
   }

   auto_ptr<CharEncSQLQuery> sqlQuery ( m_sqlConnection->newQuery() );

   {
      // do query
      MC2String query = "SELECT staticID,poiID FROM POIStatic WHERE poiID IN ";
      ::addIDs( query, beginIDs, endIDs );
   
      if ( ! doQuery(sqlQuery.get(), query.c_str(),
                     "GetAdditionalPOIInfo::getStaticIDs") ) {
         return -1;
      }
   }


   // save old end of ids, which will be the start of the new ids we are about
   // to add in this request
   uint32 idStart = ids.size(); 

   // resize it to fit all ids ( note: ids.size() is needed because we might
   // have a split request, see begining of function )
   ids.resize( ids.size() + numberOfIDs );
   // add ids from the query
   while ( sqlQuery->nextRow() ) {
      uint32 poiID = StringConvert::convert<uint32>(sqlQuery->getColumn(1));
      // find poiID oxffset in map and assign it to static ID
      ids[ idStart + idMap[ poiID ] ] = 
         StringConvert::convert<uint32>(sqlQuery->getColumn(0));
   }

   return 0;

} catch ( const StringConvert::ConvertException& e ) {
   mc2dbg << fatal << "[GetAdditionalPOIInfo] getStaticIDs " 
          << e.what() << endl;
   return -1;
}

int
GetAdditionalPOIInfo::
precache( const uint32* beginIDs,
          const uint32* endIDs,
          const set<GetAdditionalPOIInfo::keyID_t>& wantedTypes,
          poiResultMap_t& result )
{
   {
      // Limit number of requests.
      uint32 nbr = endIDs - beginIDs;
      if ( nbr >
           Properties::getUint32Property("POI_SQL_MAX_PRECACHE", 10000 ) ) {
         const uint32* middleIDs = beginIDs + nbr / 2;
         return precache( beginIDs, middleIDs, wantedTypes, result ) + 
            precache( middleIDs, endIDs, wantedTypes, result );
      } else if ( nbr == 0 ) {
         return 0;
      }
   }
   
   MC2String query = "SELECT poiID,keyID,val,lang FROM POIInfo WHERE poiID IN ";
   ::addIDs( query, beginIDs, endIDs );

   if ( ! wantedTypes.empty() ) {
      query += " AND keyID IN (";
      for( set<keyID_t>::const_iterator it = wantedTypes.begin();
           it != wantedTypes.end();
           ++it ) {
         if ( it != wantedTypes.begin() ) {
            query += ", ";
         }
         char tmp[12];
         sprintf( tmp, "%u", (unsigned int)*it);
         query += tmp;
      }
      query += ")";
   }

   mc2dbg8 << "[GAPI]::precache : query = " << MC2CITE( query ) << endl;
   
   auto_ptr<CharEncSQLQuery> sqlQuery ( m_sqlConnection->newQuery() );
   
   if ( ! doQuery(sqlQuery.get(), query.c_str(),
                  "GetAdditionalPOIInfo::precache") ) {
      return -1;
   }

   // Then add the results to the cache.
   int nbrAdded = 0;
   while (sqlQuery->nextRow()) try {
      // Is a number, no conversion should be necessary
      const char* idStr = sqlQuery->getRawColumn(0);
      // Is a number, no conversion should be necessary
      const char* keyStr = sqlQuery->getRawColumn(1);
      // Is a string
      const char* value = sqlQuery->getColumn(2);
      // number
      const char* langStr = sqlQuery->getColumn( 3 );

      uint32 id = StringConvert::convert<uint32>( idStr );
      uint32 lang = StringConvert::convert<uint32>( langStr );
      keyID_t key = keyID_t( StringConvert::convert<uint32>( keyStr ) );
                             
      mc2dbg2 << "[GAPI]: poi = " << id
              << ", key = " << (int)key
              << ", val = " << value 
             << ", lang = " << lang << endl;
      result[ id ].insert( make_pair( key, make_pair( value, lang ) ) );
      ++nbrAdded;

   } catch( const StringConvert::ConvertException& e ) {
      mc2log << warn << "[GetAdditionalPOIInfo] precache: " 
             << e.what() << endl;
   }

   return nbrAdded;
}

int
GetAdditionalPOIInfo::getInfo(uint32 sqlID, resultMap_t& data )
{
   if (!checkDatabaseConnection()) {
      return -1;
   }
   char query[500];
   sprintf(query, "SELECT keyID, val, lang FROM POIInfo WHERE poiID=%u;", sqlID);
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   if ( ! doQuery(sqlQuery, query, "GetAdditionalPOIInfo::getInfo") ) {
      delete sqlQuery;
      return -1;
   }
   int nbrAdded = 0;
   while (sqlQuery->nextRow()) try {
      const char* key = sqlQuery->getColumn(0);
      keyID_t keyID = keyID_t( StringConvert::convert<uint32>( key ) );
      uint32 lang = StringConvert::convert<uint32>( sqlQuery->getColumn( 2 ) );
      const char* val = sqlQuery->getColumn(1);
      mc2dbg8 << "Got key/value-pair for poi with id " << sqlID << ": "
              << key << "::" << val << endl;
      
      data.insert( make_pair(keyID, make_pair( val, lang ) ) );
      ++nbrAdded;
   } catch ( const StringConvert::ConvertException& e ) {
      mc2log << warn << "[GetAdditionalPOIInfo] getInfo: " << e.what() << endl;
   }

   delete sqlQuery;
   return nbrAdded;
}

const char* 
GetAdditionalPOIInfo::getKeyString( uint32 keyID,
                                    LangTypes::language_t lang,
                                    const char* defaultValue ) const
{
   // If the key is represented in StringTable, use that in correct 
   // language!
   if ( ( keyID < getNbrKeys() ) && 
        ( ItemInfoEnums::infoTypeToStringCode( getType( keyID ) )
          != StringTable::NOTOK) ) {
     
      StringTable::languageCode langCode = 
         ItemTypes::getLanguageTypeAsLanguageCode( lang );
      // Set to ENGLISH if invalid language
      if ( langCode == StringTable::SMSISH_ENG ) {
         langCode = StringTable::ENGLISH;
      }
      
      return StringTable::getString( 
         ItemInfoEnums::infoTypeToStringCode( getType( keyID ) ), langCode); 
   }

   // No data in enumeration, reply in english
   return defaultValue;
}

ItemInfoEnums::InfoType
GetAdditionalPOIInfo::getType( uint32 keyID ) const {
   if ( keyID < getNbrKeys() ) {
      return m_keyID_to_type[keyID];
   }
   // No data in enumeration, reply with text
   return ItemInfoEnums::text;
}


class POIInfoDataFull {
   public:
   POIInfoDataFull( LangTypes::language_t language,
                MC2String value, MC2String description ) 
         : lang( language ), val( value ), desc( description )
   {}

   LangTypes::language_t lang;
   MC2String val;
   MC2String desc;
};

typedef multimap< uint32, POIInfoDataFull > poiInfoDataMap;


const POIInfoDataFull& getBestPOIInfoDataFull( const poiInfoDataMap& infos, uint32 keyID,
                                       LangTypes::language_t reqlang )
{
   MC2_ASSERT( !infos.empty() );
   pair< poiInfoDataMap::const_iterator, poiInfoDataMap::const_iterator > res =
      infos.equal_range( keyID );
   vector< uint32 > stringInfos;
   vector< poiInfoDataMap::const_iterator > stringIdx;

   for ( poiInfoDataMap::const_iterator it = res.first ; it != res.second ;
         ++it )
   {
      uint32 si = CREATE_NEW_NAME( it->second.lang, 
                                   ItemTypes::officialName,
                                   stringInfos.size() );
      stringInfos.push_back( si );
      stringIdx.push_back( it );
   }
   int idx = NameUtility::getBestName( stringInfos.size(), 
                                       &stringInfos.front(), reqlang );
   if ( idx != -1 ) {
      // Use the index
      return stringIdx[ idx ]->second;
   } else {
      // Ehhh, use first
      return res.first->second;
   }
}

bool 
GetAdditionalPOIInfo::addPOIInfo( ItemInfoData& reply,
                                  uint32 keyID, const POIInfoDataFull& info,
                                  LangTypes::language_t reqLang,
                                  ItemInfoEnums::InfoTypeFilter infoFilterLevel ) const
{
   ItemInfoEnums::InfoType type = getType( keyID );
   if ( !ItemInfoFilter::includeItemInfo( type, infoFilterLevel ) ) {
      // This item info should not be added
      return false;
   }

   const char* keyString = getKeyString( keyID, reqLang, 
                                         info.desc.c_str() );
   const char* valString = info.val.c_str();
   if ( m_keyID_value_is_bool[ keyID ] ) {
      // Change value from "1"/"0" to "YES"/"NO"
      StringTable::languageCode langCode = 
         ItemTypes::getLanguageTypeAsLanguageCode( reqLang );
      // Set to ENGLISH if invalid language
      if ( langCode == StringTable::SMSISH_ENG ) {
         langCode = StringTable::ENGLISH;
      }
      if ( info.val == "0" ) {
         // NO
         valString = StringTable::getString( StringTable::NO, langCode );
      } else if ( info.val == "1" ) {
         // YES
         valString = StringTable::getString( StringTable::YES, langCode);
      } // Else not boolean use value as is
   }
   mc2dbg8 << "Got key/value-pair "
           << keyString << "::" << valString << ", type="
           << uint32(type) << endl;
   reply.addInfoEntry( 
      keyString, 
      STLStringUtility::breakLine( valString, 80/*Good?*/ ).c_str(), 
      type );

   return true;
}

int GetAdditionalPOIInfo::addToPacket( const GenericMap& map, 
                                       const PointOfInterestItem* item,
                                       ItemInfoData& reply,
                                       LangTypes::language_t lang,
                                       const char* addrString,
                                       ItemInfoEnums::InfoTypeFilter infoFilterLevel )
{
   auto_ptr<POIInfo> poiInfoClean( map.getPOIInfo( item ) );
   // no info, or something is wrong
   if ( poiInfoClean.get() == NULL ) {
      return -1;
   }

   // if the poi contains dynamic information, it should be fetch
   // from another table
   if ( poiInfoClean->hasDynamicInfo() ) {
      if ( addDynamicInfo( item->getWASPID(), *poiInfoClean ) ) {
         mc2log << warn << "[GetAdditionalPOIInfo::addToPacket] Not adding "
                << " dynamic information for waspID " << item->getWASPID()
                << ". Reason: database failure." << endl;
      }
   }

   // a set of key types the client are only interested in
   uint32 onlyTheseTypesData[] = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
      15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,
      30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,
      45,46,48,49,50,51,52,53,54,55,56,
      60,61,62,63,64,65,66,67,68,69,
      73,78,79,
      86,
      89, 96, 97, 98,
      101, 
      103, 104, // Booking
   };

   POIInfo::DataTypeSet onlyTheseTypes( onlyTheseTypesData, 
                                        onlyTheseTypesData + 
                                        sizeof( onlyTheseTypesData ) / 
                                        sizeof( onlyTheseTypesData[ 0 ] ) );

   // create a new poi info and filter the information according
   // to the table above
   auto_ptr<POIInfo> poiInfo( new POIInfo( *poiInfoClean, onlyTheseTypes ) );

   poiInfoDataMap infos;
   int nbrAdded = 0;

   for ( uint32 i = 0, n = poiInfo->getInfos().size(); i < n; ++i ) {
      POIInfoData* data = poiInfo->getInfos()[ i ];
      uint32 keyID = data->getType();
      ItemInfoEnums::InfoType type = getType( data->getType() );
      if ( m_keyID_value_is_bool[ keyID ] &&
           data->getInfo().size() == 1 && data->getInfo()[ 0 ] == '0' ) {
         // No plain "0"
         mc2log << info << "GetAdditionalPOIInfo::addToPacket skipping "
                << "info with \"0\" value. Key " << data->getType() << " val " 
                << data->getInfo() << " type " << uint32(type)
                << endl;
         continue;
      }
      if ( keyID == START_DATE || keyID == END_DATE || 
           keyID == START_TIME || keyID == END_TIME ) {
         char* digitsStr = StringUtility::removeAllButDigits( data->
                                                              getInfo().c_str() );
         uint32 intVal = atoi( digitsStr );
         delete [] digitsStr;
         if ( intVal == 0 ) {
            mc2log << info << "GetAdditionalPOIInfo::addToPacket skipping "
                   << "start/end info with 0 as value. Key " << data->getType() 
                   << " val " << data->getInfo() << " type "
                   << uint32(type) << endl;
            continue;
         }
      }

      const char* infoTypeStr = ItemInfoEnums::
         infoTypeToString( data->getLang(), getType( data->getType() ), "" );

      infos.
         insert( make_pair( data->getType(),
                            POIInfoDataFull( data->getLang(), data->getInfo(),
                                             infoTypeStr ) ) );
   }

   bool moreInfoExists = false;

   // Address
   // If 6 then use it else if 0,1 use them.
   if ( infos.find( 6 ) != infos.end() ) {
      if ( addPOIInfo( reply, 6, getBestPOIInfoDataFull( infos, 6, lang ), lang, 
                       infoFilterLevel ) ) {
         nbrAdded++;
      } else {
         moreInfoExists = true;
      }
   } else if ( infos.find( 0 ) != infos.end() ) {
      if ( infos.find( 1 ) != infos.end() ) {
         // Fixme: Add house nbr to 0:s
         if ( addPOIInfo( reply, 1, getBestPOIInfoDataFull( infos, 1, lang ), lang, 
                          infoFilterLevel) ) {
            nbrAdded++;
         } else {
            moreInfoExists = true;
         }
      }
      if ( addPOIInfo( reply, 0, getBestPOIInfoDataFull( infos, 0, lang ), lang, 
                       infoFilterLevel ) ) {
         nbrAdded++;
      } else {
         moreInfoExists = true;
      }
   } else if ( addrString != NULL ) {
      // Use addrString
      POIInfoDataFull info( lang, addrString, "Address" );
      if ( addPOIInfo( reply, 0, info, lang, infoFilterLevel ) ) {
         nbrAdded++;
      } else {
         moreInfoExists = true;
      }
   }
   // Remove all 0,1,6 from infos
   infos.erase( 0 );
   infos.erase( 1 );
   infos.erase( 6 );
   // Image link
   // If keyID is 48 (Image) turn into image link.
   if ( infos.find( 48 ) != infos.end() ) {
      POIInfoDataFull& info = infos.find( 48 )->second;
      // Change into Link
      MC2String tmpStr = getImageLink( info.val );
      if ( tmpStr[ 0 ] != '\0' ) {
         info.val = tmpStr;
         if ( addPOIInfo( reply, 48, info, lang, infoFilterLevel ) ) {
            nbrAdded++;
         } else {
            moreInfoExists = true;
         }
      } // Else no image
   }
   // Remove all 48
   infos.erase( 48 );

   // Mountain height min/max meters (keyID 82)
   //| 61 | Ski mountain max height       |
   //| 62 | Ski mountain min height       |
   if ( (infos.find( 61 ) != infos.end()) ||
        (infos.find( 62 ) != infos.end()) ) {
      MC2String min = "";
      if ( infos.find( 62 ) != infos.end() ) {
         min = (infos.find( 62 ))->second.val;
      }
      MC2String max = "";
      if ( infos.find( 61 ) != infos.end() ) {
         max = (infos.find( 61 ))->second.val;
      }
      POIInfoDataFull info( lang, getMinMaxString(min, max),
                        "Mountain height (min/max m)" );
      if ( addPOIInfo( reply, 82, info, lang, infoFilterLevel ) ) {
         nbrAdded++;
      } else {
         moreInfoExists = true;
      }
      
   }
   infos.erase( 61 );
   infos.erase( 62 );
   
   // Snow depth valley/mountain cm (keyID 83)
   //| 63 | Snow depth mountain           |
   //| 64 | Snow depth valley             |
   if ( (infos.find( 63 ) != infos.end()) ||
        (infos.find( 64 ) != infos.end()) ) {
      MC2String min = "";
      if ( infos.find( 64 ) != infos.end() ) {
         min = (infos.find( 64 ))->second.val;
      }
      MC2String max = "";
      if ( infos.find( 63 ) != infos.end() ) {
         max = (infos.find( 63 ))->second.val;
      }
      POIInfoDataFull info( lang, getMinMaxString(min, max),
                        "Snow depth (valley/mountain cm)" );
      if ( addPOIInfo( reply, 83, info, lang, infoFilterLevel ) ) {
         nbrAdded++;
      } else {
         moreInfoExists = true;
      }
   }
   infos.erase( 63 );
   infos.erase( 64 );

   // Lifts (open/total) (keyID 84)
   //| 66 | Lifts total                   |
   //| 67 | Lifts open                    |
   if ( (infos.find( 66 ) != infos.end()) ||
        (infos.find( 67 ) != infos.end()) ) {
      MC2String min = "";
      if ( infos.find( 67 ) != infos.end() ) {
         min = (infos.find( 67 ))->second.val;
      }
      MC2String max = "";
      if ( infos.find( 66 ) != infos.end() ) {
         max = (infos.find( 66 ))->second.val;
      }
      POIInfoDataFull info( lang, getMinMaxString(min, max),
                        "Lifts (open/total)" );
      if ( addPOIInfo( reply, 84, info, lang, infoFilterLevel ) ) {
         nbrAdded++;
      } else {
         moreInfoExists = true;
      }
   }
   infos.erase( 66 );
   infos.erase( 67 );
   
   // Slopes (open/total km) (keyID 85)
   //| 68 | Slopes total km               |
   //| 69 | Slopes open km                |
   if ( (infos.find( 68 ) != infos.end()) ||
        (infos.find( 69 ) != infos.end()) ) {
      MC2String min = "";
      if ( infos.find( 69 ) != infos.end() ) {
         min = (infos.find( 69 ))->second.val;
      }
      MC2String max = "";
      if ( infos.find( 68 ) != infos.end() ) {
         max = (infos.find( 68 ))->second.val;
      }
      POIInfoDataFull info( lang, getMinMaxString(min, max),
                            "Slopes (open/total km)" );
      if ( addPOIInfo( reply, 85, info, lang, infoFilterLevel ) ) {
         nbrAdded++;
      } else {
         moreInfoExists = true;
      }
   }
   infos.erase( 68 );
   infos.erase( 69 );

   // For all
   MC2String staticIDStr;
   STLStringUtility::uint2str( poiInfo->getStaticID(), staticIDStr );
   infos.insert( make_pair( 87, 
                            POIInfoDataFull( LangTypes::english,
                                             staticIDStr,
                                             // Hardcoded stringto allow client 
                                             // to recognize
                                             "static_id" ) ) );

   for ( uint32 i = 0 ; i < getNbrKeys() ; ++i ) {
      if ( infos.find( i ) != infos.end() ) {
         if ( addPOIInfo( reply, i, 
                          getBestPOIInfoDataFull( infos, i, lang ), 
                          lang, infoFilterLevel ) ) {
            nbrAdded++; 
         } else {
            moreInfoExists = true;
         }
      }
   }

   reply.setMoreInfoAvailable( moreInfoExists );

   mc2dbg4 << "Nbr added: " << nbrAdded << endl;
   return nbrAdded;
}

bool
GetAdditionalPOIInfo::doQuery(CharEncSQLQuery* sqlQuery, 
                              const char* query, 
                              const char* whereTag)
{
   // XXX: Please notice that this method is copied directly from 
   //      UserProcessor!

   mc2dbg8 << "GetAdditionalPOIInfo::doQuery(), query: " << query
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


MC2String 
GetAdditionalPOIInfo::getImageLink( MC2String image ) const {
   if ( image.empty() ||
        Properties::getProperty( "POI_IMAGE_DIR_URI" ) == NULL )
   {
      return "";
   }
   if ( strncmp( image.c_str(), "http", 4 ) == 0 ) {
      // Already an url so no need to link it to POI_IMAGE_DIR_URI
      return image;
   }
   const char* uri = Properties::getProperty( "POI_IMAGE_DIR_URI" );

   // Make dir
   char dir[50];
   MC2String tmp( image );
   MC2String::size_type pos = tmp.rfind( '.');
   if ( pos != MC2String::npos ) {
      tmp.erase( pos, tmp.size() );
   }
   // Padd with 0's
   if ( tmp.size() < 10 ) {
      tmp.insert( 0, MC2String( 10-tmp.size(), '0' ) );
   }
   uint32 intA = atoi( tmp.substr( 7, 3 ).c_str() ) / 100;
   uint32 intB = atoi( tmp.substr( 4, 3 ).c_str() ) / 100;
   uint32 intC = atoi( tmp.substr( 0, 4 ).c_str() ) / 100;
   sprintf( dir, "%u/%u/%u", intA, intB, intC );

   return MC2String(uri) + dir + "/" + image;
}
