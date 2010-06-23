/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GETADDITIONALPOIINFO_H
#define GETADDITIONALPOIINFO_H


#include "config.h"
#include "SQLDriver.h"
#include <vector>
#include <set>
#include "MC2String.h"
#include "ItemInfoPacket.h"
#include "Item.h"
#include "PointOfInterestItem.h"

class CharEncSQLConn;
class CharEncSQLQuery;
class POIInfoData;
class GenericMap;
class POIInfoDataFull;
class POIInfo;


/**
 *
 *
 */
class GetAdditionalPOIInfo {

   public:
      /**
       *    The key ids.
       */
      enum keyID_t {
         VIS_ADDRESS              = 0,
         VIS_HOUSE_NBR            = 1,
         VIS_ZIP_CODE             = 2,
         VIS_COMPLETE_ZIP         = 3,
         PHONE                    = 4,
         VIS_ZIP_AREA             = 5,
         VIS_FULL_ADDRESS         = 6,
         FAX                      = 7,
         EMAIL                    = 8,
         URL                      = 9,
         BRANDNAME                = 10,
         SHORT_DESCRIPTION        = 11,
         LONG_DESCRIPTION         = 12,
         CITYPART                 = 13,
         STATE                    = 14,
         NEIGHBORHOOD             = 15,
         OPEN_HOURS               = 16,
         NEAREST_TRAIN            = 17,
         START_DATE               = 18,
         END_DATE                 = 19,
         START_TIME               = 20,
         END_TIME                 = 21,
         ACCOMMODATION_TYPE       = 22,
         CHECK_IN                 = 23,
         CHECK_OUT                = 24,
         NBR_OF_ROOMS             = 25,
         SINGLE_ROOM_FROM         = 26,
         DOUBLE_ROOM_FROM         = 27,
         TRIPLE_ROOM_FROM         = 28,
         SUITE_FROM               = 29,
         EXTRA_BED_FROM           = 30,
         WEEKEND_RATE             = 31,
         NONHOTEL_COST            = 32,
         BREAKFAST                = 33,
         HOTEL_SERVICES           = 34,
         CREDIT_CARD              = 35,
         SPECIAL_FEATURE          = 36,
         CONFERENCES              = 37,
         AVERAGE_COST             = 38,
         BOOKING_ADVISABLE        = 39,
         ADMISSION_CHARGE         = 40,
         HOME_DELIVERY            = 41,
         DISABLED_ACCESS          = 42,
         TAKEAWAY_AVAILABLE       = 43,
         ALLOWED_TO_BRING_ALCOHOL = 44,
         TYPE_FOOD                = 45,
         DECOR                    = 46,
         DISPLAY_CLASS            = 47,
         IMAGE                    = 48,
         SUPPLIER                 = 49,
         OWNER                    = 50,
         PRICE_PETROL_SUPERPLUS   = 51,
         PRICE_PETROL_SUPER       = 52,
         PRICE_PETROL_NORMAL      = 53,
         PRICE_DIESEL             = 54,
         PRICE_BIODIESEL          = 55,
         FREE_OF_CHARGE           = 56,
         POIMAIN_RIGHTS           = 57,
         MYCHILD                  = 58,
         MYPARENT                 = 59,
         OPEN_SEASON              = 60,
         SKI_MOUNTAIN_MAX_HEIGHT  = 61,
         SKI_MOUNTAIN_MIN_HEIGHT  = 62,
         SNOW_DEPTH_MOUNTAIN      = 63,
         SNOW_DEPTH_VALLEY        = 64,
         SNOW_QUALITY             = 65,
         LIFTS_TOTAL              = 66,
         LIFTS_OPEN               = 67,
         SLOPES_TOTAL_KM          = 68,
         SLOPES_OPEN_KM           = 69,
         VALLEY_RUN               = 70,
         VALLEY_RUN_INFO          = 71,
         CROSS_COUNTRY_SKIING     = 72,
         CROSS_COUNTRY_SKIING_TOTAL_KM = 73,
         FUNPARK                  = 74,
         FUNPARK_INFO             = 75,
         NIGHT_SKIING             = 76,
         NIGHT_SKIING_INFO        = 77,
         GLACIER_AREA             = 78,
         LAST_SNOWFALL            = 79,
         HALFPIPE                 = 80,
         HALFPIPE_INFO            = 81,

         // follows the POIInfoKeys table strictly
         
         NBR_KEYS
      };

   typedef set<keyID_t> keyIDSet_t;

   typedef multimap<keyID_t, pair<MC2String, uint32> > resultMap_t;

   typedef map<uint32, resultMap_t> poiResultMap_t;

      /**
       *    Create a new object of this class to get additional information 
       *    from the SQL-database about the poi:s. Will connect to the 
       *    database that is specified as parameters. The constructor will 
       *    make the necessary connections to the database, these are used
       *    by the other methods during the life time of the object.
       *
       *    @warn Currently only handles connections to MySQL.
       *
       *    @param database      The name of the database to use.
       *    @param host          The host where to connect to MySQL.
       *    @param username      Username when connecting to the database.
       *    @param password      Password when connection to the database.
       */
      GetAdditionalPOIInfo(const char* database,
                           const char* host,
                           const char* username,
                           const char* password);

      virtual ~GetAdditionalPOIInfo();
   
      /**
       *    Get all additional information about one point ot interest. 
       *    Data will be added to the vector that is sent as parameter.
       *    @param sqlID   The primary key of the POI in the wasp-database.
       *    @param data    Outparameter where the data is stored. Added 
       *                   as key / value pair.
       *    @return  True if a POI with the given sqlID was found, false
       *             otherwise.
       */
      int getInfo(uint32 sqlID, resultMap_t& data);
      /**
       *    Executes the query and adds the data in the map.
       *    @param query The query to be executed.
       *    @param data Outparameter where the data is stored.
       *    @return  The number of added pieces of data.
       */
      int getInfo( const GenericMap& map,
                   vector<Item*>::iterator lower,
                   vector<Item*>::iterator upper,
                   const set<GetAdditionalPOIInfo::keyID_t>& keyIDs,
                   poiResultMap_t &data );
      
   /**
    * Adds information about a specific POI item to a packet.
    * 
    * @param map The map in which the POI item resides in.
    * @param item The POI item that information should be retreived for 
    * @param reply The item to be filled in with information about the 
    *        POI Item
    * @param lang Requested language of the information.
    * @param addrString An address string for the item.
    * @param infoFilterLevel The filter level to use when adding infos
    * @return -1 on failure.
    */
   int addToPacket( const GenericMap& map,  
                    const PointOfInterestItem* item,
                    ItemInfoData& reply,
                    LangTypes::language_t lang,
                    const char* addrString,
                    ItemInfoEnums::InfoTypeFilter infoFilterLevel );
      /**
       *    Use this function to pre-cache data when you know what
       *    you want later. Use with care since the cache is never emptied
       *    automatically.
       */
      int getMultiInfo( const set<uint32>& waspIDs,
                        const keyIDSet_t& wantedTypes,
                        poiResultMap_t& res);
      

   /**
    * Fetches information for a range of POIs.
    * @param beginIDs start of POI range
    * @param endIDs end of POI range
    * @param wantedTypes key types the client wants, leave empty if all key IDs
    *        are important.
    * @param result information about POIs mapped to the id 
    * @return -1 on failure
    */
   int precache( const uint32* beginIDs,
                 const uint32* endIDs,
                 const set<GetAdditionalPOIInfo::keyID_t>& wantedTypes,
                 poiResultMap_t& result );
   /**
    * Gets static ids from a range of wasp ids
    * @param beginIDs start of wasp id range
    * @param endIDs end of wasp id range
    * @param ids the vector to be filled with static ids for specified range of
    *        wasp ids
    * @return -1 on failure
    */
   int getStaticIDs( const uint32* beginIDs,
                     const uint32* endIDs,
                     vector<uint32>& ids );

   /**
    * Gets ids which holds dynamic information.
    * @param ids a set to be filled in with ids that resides in the dynamic
    *        information table.
    * @return -1 on failure.
    */
   int getDynamicIDs( set<uint32>& ids );


private:
   /**
    * Fetches and adds dynamic information to POIInfo for a specific POI Item.
    * @param dbID the database ID that the client wants information about
    * @param info information object to contain the dynamic information 
    * @return -1 on failure.
    */
   int addDynamicInfo( uint32 dbID, POIInfo& info );


      static uint32 getNbrKeys();

      /**
       *    Copyed from UserProcessor!
       */
      bool doQuery(CharEncSQLQuery* sqlQuery,
                   const char* query, const char* whereTag);

      /**
       *    Check if we have a valid connection to the database. If not, 
       *    the method will try to reconnect maxNbrTries times.
       *    @param maxNbrTries   The maximum number of times we will try to
       *                         reconnect to the database before giving up.
       *    @return  True if it is safe to use m_sqlConnection after this 
       *             call. Something is seriously wrong if this method return
       *             false!
       */
      bool checkDatabaseConnection(uint32 maxNbrTries = 1);

      const char* getKeyString( uint32 keyID,
                                LangTypes::language_t lang,
                                const char* defaultValue ) const;


      /**
       * Turns an image value into a link.
       *
       * @param image The image value.
       * @return The link to the image.
       */
      MC2String getImageLink( MC2String image ) const;

      ItemInfoEnums::InfoType getType( uint32 keyID ) const;

      /**
       * Add an info field to reply.
       *
       * @param reply The item to add to.
       * @param keyID The key string.
       * @param info The information.
       * @param reqLang The prefered language.
       * @param infoFilterLevel The filter level to use when adding info
       * @return True if the info is actually added
       */
      bool addPOIInfo( ItemInfoData& reply, 
                       uint32 keyID, const POIInfoDataFull& info,
                       LangTypes::language_t reqLang,
                       ItemInfoEnums::InfoTypeFilter infoFilterLevel ) const;

      /**
       *    The connection to the database.
       */
      CharEncSQLConn* m_sqlConnection;

      /**
       *    Keep tracks of the number of information requests to ignore until
       *    it is time to try reconnect do the database again.
       */
      int m_waitDBRetry;

      char* m_database;
      char* m_host;
      char* m_username;
      char* m_password;

      /**
       *    Array to map infoKeyID in the database with 
       *    StringTable::stringCode.
       */
      static ItemInfoEnums::InfoType m_keyID_to_type[];
      // If a value is bool (0->"No",1->"Yes"), int for future. 
      static int m_keyID_value_is_bool[];
};

#endif

