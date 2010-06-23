/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPGENUTIL_H
#define MAPGENUTIL_H


#include <map>
#include <set>

#include "config.h"
#include "MC2String.h"
#include "MapGenEnums.h"
#include "StringTable.h"

class OldGenericMap;
class OldOverviewMap;

/**
 *   Class used for map generation specific tasks not needed by the 
 *   mc2 server. Use MC2MapGenUtil if the server needs access.
 *
 */
class MapGenUtil {
 public:
   /**
    * Returns the next available map ID in the directory
    * of mapPath for each map type respectively
    */ 
   //@{
   static uint32 getNextSupOvrMapID( MC2String mapPath );
   static uint32 getNextCoOvrID( MC2String mapPath );
   static uint32 getNextOvrMapID( MC2String mapPath );
   static uint32 getNextUndMapID( MC2String mapPath );
   
   //@}

   /**
    * @param gmsName String containing the GMS name of a country.
    * @return The country code corresponding to the given GMS name.
    *         Returns NBR_COUNTRY_CODES if the GMS name is invalid.
    */
   static StringTable::countryCode 
      getCountryCodeFromGmsName( const MC2String& gmsName );


   
   /**
    *    Returns the map ids of files found in the mapPath directory.
    *
    *    @param ids Outparameter The IDs will be returned here.
    *    @param mapPath The directory to look for map files in.
    */
   static void getMapIDsInDir(set<uint32>& ids, MC2String mapPath);

   /**
    * Returns the ID of the map with the lowest map ID of the maps present
    * in mapPath.
    */
   //@{
   static uint32 firstExistingCoOvrMapID( MC2String mapPath );
   //@}
  
   /** @return Returns the first map ID used by extra item maps. Use 
    *          MapBits::nextMapID to get the next
    */
   static uint32 getExtItemFirstMapID(){
      return 0x2001;
   }

   /**
    * @param  ovrMapID An overview map.
    * @return Returns the underview map IDs of this overview map by loading
    *         and checking the list in the country overview map.
    */
   static vector<uint32> getAllUndMapIDs(const OldOverviewMap* ovrMap);



   /**
    * Map version and supplier handling.
    */
   //@{

   /** Includes map supplier company and map version. One map version
    *  can only exist together with one specific  map supplier.
    */
   typedef pair<MapGenEnums::mapSupplier, MapGenEnums::mapVersion> 
      mapSupAndVerPair;
   
   /**
    *   @param  gdfMapOrigin String containing both map supplier and 
    *                        map version. The map supplier is separated
    *                        from the map version by the first '_'.
    *   @return Returns map supplier and map version from a map origin
    *           string. If map supplier can't be defined, 
    *           MapGenEnums::unknownSupplier is returned. If map origin
    *           can't be defined, MapGenEnums::invMapVersion is returned.
    */
   static mapSupAndVerPair getMapSupAndVer(const char* gdfMapOrigin);

   /**
    *   @param  gdfMapOrigin String containing both map supplier and 
    *                        map version. The map supplier is separated
    *                        from the map version by the first '_'.
    *   @return Returns map supplier from a map origin string. If map
    *           supplier can't be defined, MapGenEnums::unknownSupplier
    *           is returned.
    */
   static MapGenEnums::mapSupplier
      getMapSupplier(const char* gdfMapOrigin);

   /**
    *    @param  mapSupStr    A string only containing the map supplier.
    *    @return Returns MapGenEnums::unknownSupplier fi the map supplier
    *            string could not be interpretted.
    */
   static MapGenEnums::mapSupplier
      getMapSupFromString( MC2String mapSupStr );
   
   /**
    *    @param  mapSup  The map supplier to get the ID string of.
    *    @return Returns empty string or map supplier ID string. The ID string
    *            is the string used in copyright box XML files.
    */
   static MC2String getIdStringFromMapSupplier(MapGenEnums::mapSupplier 
                                               mapSup);

   /**
    *    @param mapSup The map supplier. Needed because same map 
    *                  version string results in different map versions
    *                  depending on map supplier.
    *    @param mapVerStr A string only containing the map version.
    *    @return Returns the map version corresponging to the str and 
    *            mapSup parameters. If none was found it returns 
    *            MapGenEnums::invMapVersion
    */
   static MapGenEnums::mapVersion 
      getMapVerFromString( MapGenEnums::mapSupplier mapSup,
                           MC2String mapVerStr );

   //@}

 
  
   /**
    * Prefix to use when writing POIs to log file to use for
    * inserting to WASP database.
    */
   static const MC2String poiWaspPrefix;
 
   /**
    * Delimiter to use when writing POIs to log file to use for
    * inserting to WASP database.
    */
   static const MC2String poiWaspFieldSep;
   static const MC2String poiWaspNameSep;
   static const MC2String poiWaspSubNameSep;

   /** Used by locality index. Here you find what directory to find the data 
    *  of each in.
    */
   static const MC2String dirBySectionFileName;



   /**
    *   Prints the filename in the map. Put here to go around 
    *   private declaration.
    *    
    *   Filename is absolute path + mapid printed in 9 positions 
    *   (zeroes in the begining) + ".mcm" at the end.
    *
    *   @param theMap The map to update filename of
    *   @param path   The file name to set.
    */
   static void setFileName(OldGenericMap* theMap, const char* path);

   /**
    *   Looks for a file called fileName on disk.
    *   @param fileName Path and name of the file.
    *   @return True if the file exists on disc, false otherwise.
    */
   static bool fileExists(MC2String fileName);


   /**
    *    Reads a file and puts its content in MC2Strings, row by row.
    *    (Maybe this method should be in a file utility)
    *
    *    @param filePath File name and path.
    *    @param rowStrings Outparameter. The rows of the file are 
    *                      appended to this vector. One row in each 
    *                      vector elment.
    */
   static bool getRowsFromFile(MC2String filePath, 
                               vector<MC2String>& rowStrings);

 
   /**
    *    Reads the content of a text file with text data in columns and
    *    rows. Puts each cell in a MC2String organized in nested vectors.
    *
    *    @param fileName Name and path to the file to load.
    *    @param nbrCols  Number of columns in the file. Used for sanity
    *                    check.
    *    @param colSeparator The sign or signs used for separating colunn
    *                        values.
    *    @param tableContent Outparameter. The result is put here. One 
    *                        vector per row containing a vector with a 
    *                        MC2String per colunn of that row.
    *    @param skipStartRows Read past skipStartRows rows at the beginning
    *                         of the file.
    *    @param skipEndRows Don't read endStartRows rows at the end of 
    *                         the file.
    *    @param prefixNbrCols Use this parameter when number of columns of 
    *                         rows differ depending on the value of column 0.
    *                         Key:   Colunn 0 (prefix) value
    *                         Value: Expected number of columns for this 
    *                                prefix.
    *    @param throwAwayCols To save memory, throw away not needed values
    *                         right away.
    *    @result True if successful, otherwise false.
    *
    */
   static bool loadTxtFileTable( MC2String fileName, 
                                 uint32 nbrCols,
                                 MC2String colSeparator,
                                 vector< vector<MC2String> >& tableContent,
                                 uint32 skipStartRows = 0,
                                 uint32 skipEndRows = 0,
                                 map<MC2String, uint32>* 
                                 prefixNbrCols = NULL,
                                 set<uint32>* throwAwayCols = NULL );

   /**
    *    Writes teh contenet of table of text to a separated row based
    *    file.
    *
    *    @param fileName Name and path to the file to save.
    *    @param nbrCols  Number of columns in the file. Used for sanity
    *                    check.
    *    @param colSeparator The sign or signs used for separating colunn
    *                        values.
    *    @param tableContent This is the data to be written to the file.
    *                        Each outer vector element is a vector 
    *                        representing row and each element in the row
    *                        vectors is representing a columnvalue of that
    *                        row.
    *
    *    @result True if successful, otherwise false.
    *
    */
   static bool saveTxtFileTable( MC2String fileName, 
                                 uint32 nbrCols,
                                 MC2String colSeparator,
                                 vector<vector<MC2String> >& tableContent);

   /**
    *    This mehtod should be called when handling case of items included in
    *    the map. Change this method to change the way case is handled.
    *
    *    @param name Outpareameter This name is modified to have right case.
    */
   static MC2String handleItemNameCase(const MC2String& name);
   
   /**
    *    Prints the bits of a bitFiled stored in a uint32. Prints the lowest
    *    nbrBits bits.
    */
   static MC2String intToBitFieldStr(uint32 bitField, uint32 nbrBits);
   

 private:
      /**
       * Helper method for findng the next available map ID on disk 
       * starting from startID. Used by methods like getNextCoOvrID, 
       * getNextOvrMapID and getNextUndMapID.
       *
       * @param startID The map ID to start looking at. Typically 0,
       *                0x80000000. or 0x80000001.
       * @param mapPath The directory to look for maps in.
       *
       * @return The next map ID with no map present in the mapPath 
       *         directory.
       */
      static uint32 getNextMapID( uint32 startID, const char* mapPath );

      /** Inits m_mapSupByMapSupName and m_mapSupNameByMapSup. Does nothing if
       *  already inited.
       */
      static void initMapSupMapping();

      /// Mapping map supplier to map supplier ID string.
      //@{
      static map<MC2String, MapGenEnums::mapSupplier> m_mapSupByMapSupName;
      static map<MapGenEnums::mapSupplier, MC2String> m_mapSupNameByMapSup;
      //@}

}; // class MapGenUtil

#endif // MAPGENUTIL_H
