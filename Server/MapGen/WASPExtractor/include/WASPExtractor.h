/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WASPEXTRACTOR_H
#define WASPEXTRACTOR_H

#include "config.h"
#include "SQLDriver.h"
#include "SQLConnection.h"
#include "OldGenericMap.h"
#include "ItemTypes.h"
#include "OldPointOfInterestItem.h"
#include "CharEncoding.h"
#include <map>

/**
 *    Get POI-data from a SQL database and insert that into the maps.
 * 
 */
class WASPExtractor {
public:
      
   struct poiAddData_t {
      uint32 dist_meters;
      uint32 mapID;
      int32 lat;
      int32 lon;
      string streetName;
   };

   struct epData_t {
      uint64 dist_sqmc2;
      uint32 mapID;  // Only for debugging
      int32 lat;
      int32 lon;
   };

   /**
    *    Create a new WASPExtractor that will connect to the database 
    *    that is specified as parameters. The constructor will make
    *    the necessary connections to the database, these are used
    *    by e.g. addToMap etc. for each map.
    *    @param database      The name of the database to use.
    *    @param host          The host where to connect to MySQL.
    *    @param username      Username when connecting to the database.
    *    @param password      Password when connection to the database.
    *    @param poiDBCharEnc  The char encoding used in the POI database.
    *    @param maxDistMeter  The maximum distance between a POI and the
    *                         closest street for the POI to be added to
    *                         the map.
    */
   WASPExtractor(const char* database,
                 const char* host,
                 const char* username,
                 const char* password,
                 CharEncodingType::charEncodingType poiDBCharEnc,
                 uint32 maxDistMeter);

   /**
    *    Delete the WASPExtractor and close the connections to the database.
    */
   virtual ~WASPExtractor();

   /**
    *    Closes and reopens all database connections
    */
   void closeThenOpenDatabaseConnection(const char* database, 
					const char* host, 
					const char* username, 
					const char* password,
               CharEncodingType::charEncodingType poiDBCharEnc);

   /**
    *    Add poi:s to the given map. Normal WASPing.
    */
   int addToMap(OldGenericMap* theMap,
                multimap<uint32, poiAddData_t>& notAddedIDs,
                uint32 mapVersionID,
                bool skipControlledAccessPOIs);

   /**
    *    Creates entry points from a given map to POIs without entry point.
    *    @param theMap  The map, which to use for createing entry points
    *                   for POIs that don't have entry points.
    *    @param epInfo  Holds entry point data when loopings maps.
    *    @param sourceString  The source string (from POISources table)
    *                   if only POIs from a given source should have 
    *                   entry points created, else NULL.
    *    @return  -1 upon failure
    */   
   int createEntryPoints(  OldGenericMap* theMap, 
                           map<uint32, epData_t>& epInfo,
                           const char* sourceString );
      
   /**
    *    Update given map with modified, new and removed POIs in the WASP.
    *    Function used for dynamic WASPing, updating maps without 
    *    regenerating them. To enable applying the updates also to 
    *    overview and country overview maps, the item ids of the updated
    *    pois are written to an out file modifiedPOIs.txt.
    *
    *    @param theMap     The map in which to update pois.
    *    @param mapVersionID  The mapVersion for which to make the update,
    *                         e.g. TeleAtlas_2003_3 (from POI.EDVersion).
    *    @param date       The date of last WASPing of the map.
    *    @param outFile    The outfile where to write mapid;itemid of
    *                      the updated pois.
    *    @param nbrNewPois Outparam, nbr of new POIs added to the map.
    *    @param nbrModifiedPois Outparam, nbr of POIs modified in the map.
    *    @param nbrRemovedPois Outparam, nbr of POIs removed from the map.
    *
    *    @return  -1 if the update fails, else the number of updated 
    *             pois in the map.
    */
   int updateInMap(OldGenericMap* theMap, uint32 mapVersionID,
                   char* date, ofstream& outFile,
                   uint32& nbrNewPois, uint32& nbrModifiedPois,
                   uint32& nbrRemovedPois,
                   bool skipControlledAccessPOIs);

   /**
    *    Check that the provided mapVersion is valid, when either
    *    adding or updating pois.
    *    @param   mapVersion  The version to check.
    *    @param   addToMap    States if the task is to addPOIToMap (true) 
    *                         or updatePOIInMap (false).
    *    @param   mapVersionID Outparam that is set to the version id 
    *                         of the mapVersion if it is valid.
    *    @return  True if the map version is valid, false if not.
    */
   bool checkValidMapVersion(const char* mapVersion, bool addToMap, 
                             uint32& mapVersionID);

   /**
    *    Get the latest wasp-date from the map header. If a valid date
    *    is stored in the map header, it is printed to a pre-allocated
    *    string accoring to "yyyy-mm-dd hh:mm:ss". This format is
    *    the one used in the WASP-database.
    *    @param   theMap   The map from which to get wasp-date.
    *    @param   waspDate String where to write the wasp-date.
    *    @return  True if the map had a valid wasp-date and it was
    *             printed to the string, false if not.
    */
   bool getWaspDateFromMap(OldGenericMap* theMap, char* waspDate);
       
   /**
    *    Get number affected pois so far.
    */
   inline uint32 getNbrAddedIds();

   /**
    *    Get the set with affected pois so far.
    */
   inline void getAddedIds(set<uint32>& addedIDs);

   /**
    *    Method that does misc things on a map. Action depends on
    *    compilation.
    */
   int waspLab(OldGenericMap* theMap, MC2String function );

   const map< MapRights, uint32 > getPOIsPerRight() const {
      return m_POIsPerRight;
   }

   /**
    *    Copyed from UserProcessor!
    */
   bool doQuery(SQLQuery* sqlQuery, const char* query, 
                const char* whereTag);
     
   SQLConnection* getSqlConnection(){
      return m_sqlConnection;
   };

private:
      
   /**
    *    Struct for storing poiNames from WASP for sorting before
    *    the names are added to the poi.
    */
   struct poiName_t {
      const char* name;
      LangTypes::language_t poiNameLang;
      ItemTypes::name_t poiNameType;
   };

   /**
    *    Struct for sorting poiNames from WASP before adding the names
    *    to the poi item. Official names must be added first,
    *    synonym names last.
    */
   struct LessPoiNameOrder:
      public binary_function<const poiName_t&, const poiName_t&, bool> {
      bool operator()(const poiName_t& x, const poiName_t& y) {
         // the order in ItemTypes::name_t is good to use
         return (int(x.poiNameType) < int(y.poiNameType));
      }
   };
      
   /**
    *    Help method to createPOI.
    *    Checks if a poi with given entry point fits the map. If it does,
    *    ssi id and offset (closest ssi) attributes of the POI is set.
    *    The entry point must be within m_maxSqDist_mc2 distance from
    *    closest ssi.
    *
    *    @param   theMap   The map in which to check if the poi fits.
    *    @param   poi      The poi to add. The poi is empty (just created)
    *                      in normal WASPing or dynamic WASPing (new),
    *                      or a true poi in dynamic WASPing (modified).
    *    @param   ssiLat   The entry point lat of the POI.
    *    @param   ssiLon   The entry point lon of the POI.
    *    @param newPOI     True if new POI, false if modified POI
    *                      in dynamic WASPing
    *    @param   notAddedData Outparam, filled with data if the POI did 
    *                      not fit the map, to be used later.
    *    @return  The POI with ssiID and offset if it fits the map,
    *             NULL if it did not fit the map.
    */
   OldPointOfInterestItem* findSsiAndOffset(OldGenericMap* theMap,
                                              OldPointOfInterestItem* poi,
                                              int32 ssiLat,
                                              int32 ssiLon,
                                              bool newPOI,
                                              poiAddData_t& notAddedData);

   /**
    *  Checkes if the POI is already added to this map
    */
      
   bool alreadyAdded(const OldPointOfInterestItem* poi, 
                     const OldGenericMap* theMap);
   /**
    *  Remove all POIs from this map
    */
      
   uint32 removeAllPOI(OldGenericMap* theMap);

   /**
    *  Returns a POI item if there is a POI with a given WASP id
    *  in this map
    */
   OldPointOfInterestItem* findPOIWithWASPID(OldGenericMap* theMap, 
                                             uint32 WASPID);
   
   /**
    *  Add values from WASP to a POI (names, type, entry point,
    *  gxf-data, WASP id). Note that this method overwrites all
    *  values that is already present in the POI.
    *  The method is used in normal WASPing as well as in dynamic WASPing
    *  (new and modified).
    *
    *  @param newPOI            True if new POI, false if modified POI
    *                           in dynamic WASPing
    *  @param notAddedToThisMap Outparameter If not added to map, this
    *                           data tells why. Is set to != 0 if the 
    *                           POI was not added.
    *  @param poiCategories     Outparameter All categories of this poi.
    *                           Set them to the item when it has got its
    *                           item ID.
    */
   OldPointOfInterestItem* createPOI(OldGenericMap* theMap,
                                     SQLQuery* subSqlQuery,
                                     OldPointOfInterestItem* poi,
                                     uint32 poiWaspID,
                                     int32 symbolLat,
                                     int32 symbolLon,
                                     bool newPOI,
                                     poiAddData_t& notAddedData,
                                     set<uint16>& poiCategories);
      
   /**
    *    Help method to createPOI.
    *    Find one entry point for a poi.
    *    @param   waspId   The database id of the poi for which 
    *                      to find entry point.
    *    @param   symbolLat   The latitude of the poi.
    *    @param   symbolLon   The longitude of the poi.
    *    @param   ssiLat   Outparam set to the entry point latitude,
    *                      same as symbolLat if no entry point exists.
    *    @param   ssiLon   Outparam set to the entry point longitude.
    *                      same as symbolLon if no entry point exists.
    *
    *    @return  True if one entry point was found, false otherwise.
    */
   bool getEntryPointForWASPID(SQLQuery* subSqlQuery, uint32 waspId,
                               int32 symbolLat, int32 symbolLon,
                               int32& ssiLat, int32& ssiLon);
      
   /**
    *    Check if the name of a poi is an alternative name, and if it
    *    is the same name as the name of either the bua or municipal 
    *    the poi is located in. In that case, the name should not be
    *    added to the poi...
    *    Note that the poi must have at least one official name with the 
    *    same language as the language of the alternative name in
    *    order to skip the alternative name.
    *
    *    Before asking this method, the poi must have poiType, and
    *    the official names must have been added.
    *
    *    @param   poi         The poi to check.
    *    @param   checkName   The name to check if it is an alternative
    *                         name equal to poi-location-names.
    *    @param   theMap      The map in which the poi is located.
    */
   bool cityCentreWithLocationAltName( OldPointOfInterestItem* poi,
                                       poiName_t checkName,
                                       OldGenericMap* theMap);

   /**
    *    The maximum distance from a POI (entry point to the closest
    *    street segment for the poi to be added. Also used as limit
    *    for when a poi should have a GfxData or not, pois with symbol
    *    coordinate within this distance from closest street segment
    *    will have no gfxData, pois more distant will have gxfData.
    */
   uint32 m_maxDist_meters;

   /**
    *    The maximum distance from a POI (entry point) to the closest
    *    street segment for the poi to be added. Based on
    *    m_maxDist_meters, but expressed in square mc2-units.
    */
   uint64 m_maxSqDist_mc2;

   /**
    *    Set holding poi waspids of all pois so far added (or modified
    *    or removed if dynamic WASPing) in any map.
    */
   set<uint32> m_addedIDs;
   /**
    *    Are we connected?
    */
   bool m_connected;

   /**
    *    The main connection to the database.
    */
   SQLConnection* m_sqlConnection;

   /**
    *    An extra connection to be able to execute sub-selects when
    *    the m_sqlConnection still is used.
    */
   SQLConnection* m_subSqlConnection;

   /**
    *    Map that is used in waspLab
    */
   map< MapRights, uint32 > m_POIsPerRight;
   
};


inline uint32
WASPExtractor::getNbrAddedIds()
{
   return m_addedIDs.size();
}

inline void
WASPExtractor::getAddedIds(set<uint32>& addedIDs)
{
   addedIDs = m_addedIDs;
}

#endif
