/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapGenUtil.h"

#include "STLStringUtility.h"
#include "StringUtility.h"
#include "OldGenericMapHeader.h"
#include "OldOverviewMap.h"
#include "OldCountryOverviewMap.h"
#include "MapBits.h"
#include "ArrayTools.h"

#include "Utility.h"

// Static definitions
const MC2String MapGenUtil::poiWaspPrefix = "YYY-";
const MC2String MapGenUtil::poiWaspFieldSep = "<�>";
const MC2String MapGenUtil::poiWaspNameSep = "<�>";
const MC2String MapGenUtil::poiWaspSubNameSep = "<�>";

const MC2String MapGenUtil::dirBySectionFileName = "sectionToDirName.txt";

map<MC2String, MapGenEnums::mapSupplier> MapGenUtil::m_mapSupByMapSupName;
map<MapGenEnums::mapSupplier, MC2String> MapGenUtil::m_mapSupNameByMapSup;


uint32 
MapGenUtil::getNextMapID( uint32 startID, const char* mapPath )
{   
   
   // Check in-parameter.
   if (mapPath == NULL){
      mc2log << error << "GMS::getNextMapID."
             << " mapPath == NULL. Exits!" << endl;
      exit(1);
   }
   if (strlen( mapPath) == 0){
      mc2log << error << "GMS::getNextMapID."
             << " strlen(mapPath) == 0. Exits!" << endl;
      exit(1);
   }
   if ( ! ( (startID == 0) || 
            (startID == 0x80000001) || 
            (startID == 0x80000000) ||
            (startID == 0x90000000) ) )
   {
      mc2log << error << "GMS::getNextMapID."
             << " invalid startID:" << startID << endl;
   }

   uint32 mapID=startID;
   
   // Loop until an ID with no file is found.
   bool foundLastID=false;
   while ( !foundLastID ){

      // Put file name and path together.
      MC2String mapFilePath = mapPath;
      if ( mapPath[strlen(mapPath)-1] != '/' ){
         mapFilePath.append("/");
      }
      MC2String idStr;
      STLStringUtility::uint2strAsHex( mapID, idStr );
      for (uint32 i=0; i<(9-idStr.size()); i++){
         // There should be 9 digits.
         mapFilePath.append("0");
      }
      mapFilePath.append(idStr);
      mapFilePath.append(".mcm");

      
      // Test to open a not compressed file.
      mc2dbg8 << "Test to open file:" << mapFilePath << endl;
      ifstream mapFile( mapFilePath.c_str() );
      foundLastID = !mapFile.is_open();
      mapFile.close();

      // Test to open a gzipped file. Perhaps we have not found the
      // last ID after all.
      if (foundLastID){
         MC2String mapFilePathGz = mapFilePath + MC2String(".gz");
         mc2dbg << "Test to open file:" << mapFilePathGz << endl;
         ifstream mapFileZip( mapFilePathGz.c_str() );
         foundLastID = !mapFileZip.is_open();
         mapFileZip.close();
      }

      // Test to open a bzip2 file. Perhaps we have not found the
      // last ID after all.
      if (foundLastID){
         MC2String mapFilePathBz2 = mapFilePath + MC2String(".bz2");
         mc2dbg << "Test to open file:" << mapFilePathBz2 << endl;
         ifstream mapFileZip( mapFilePathBz2.c_str() );
         foundLastID = !mapFileZip.is_open();
         mapFileZip.close();
      }
      
      if (!foundLastID){
         mapID = MapBits::nextMapID(mapID);
      }
   }
   return mapID;
} //  getNextMapID

uint32
MapGenUtil::getNextSupOvrMapID( MC2String mapPath ){
   return getNextMapID( 0x90000000, mapPath.c_str() );
} //  getNextSupOvrID


uint32 
MapGenUtil::getNextCoOvrID( MC2String mapPath ){
   return getNextMapID( 0x80000001, mapPath.c_str() );
} //  getNextCoOvrID

uint32 
MapGenUtil::getNextOvrMapID( MC2String mapPath ){
   return getNextMapID( 0x80000000, mapPath.c_str() );
} //  getNextOvrMapID

uint32 
MapGenUtil::getNextUndMapID( MC2String mapPath ){
   return getNextMapID( 0, mapPath.c_str() );
} //  getNextUndMapID

uint32 
MapGenUtil::firstExistingCoOvrMapID( MC2String mapPath ){
   uint32 foundMapID = MAX_UINT32;
   uint32 mapID = 0x80000001;
   uint32 tries = 0;
   while ( ( foundMapID == MAX_UINT32 ) && ( tries < 400) ) {

      // Put file name and path together.
      MC2String mapFilePath = mapPath;
      if ( mapPath[mapPath.size()-1] != '/' ){
         mapFilePath.append("/");
      }
      MC2String idStr;
      STLStringUtility::uint2strAsHex( mapID, idStr );
      for (uint32 i=0; i<(9-idStr.size()); i++){
         // There should be 9 digits.
         mapFilePath.append("0");
      }
      mapFilePath.append(idStr);
      mapFilePath.append(".mcm");
      
      
      // Test to open a not compressed file.
      bool fileExists = false;
      mc2dbg8 << "Test to open file:" << mapFilePath << endl;
      ifstream mapFile( mapFilePath.c_str() );
      fileExists = mapFile.is_open();
      mapFile.close();
      
      // Test to open a gzipped file. Perhaps we have not found the
      // last ID after all.
      if (!fileExists){
         MC2String mapFilePathGz = mapFilePath + MC2String(".gz");
         mc2dbg << "Test to open file:" << mapFilePathGz << endl;
         ifstream mapFileZip( mapFilePathGz.c_str() );
         fileExists = mapFileZip.is_open();
         mapFileZip.close();
      }
      
      // Test to open a bzip2 file. Perhaps we have not found the
      // last ID after all.
      if (!fileExists){
         MC2String mapFilePathBz2 = mapFilePath + MC2String(".bz2");
         mc2dbg << "Test to open file:" << mapFilePathBz2 << endl;
         ifstream mapFileZip( mapFilePathBz2.c_str() );
         fileExists = mapFileZip.is_open();
         mapFileZip.close();
      }
      if ( fileExists ){
         foundMapID = mapID;
      }
      
      tries++;      
      mapID = MapBits::nextMapID(mapID);      
   }

   return foundMapID;
} //  findFirstCoOvrMapID


void
MapGenUtil::getMapIDsInDir(set<uint32>& ids, MC2String mapPath)
{
   char** entries = NULL;
   int nbr = Utility::directory(mapPath.c_str(), &entries);
   if ( nbr < 0 ) {
      mc2log << error << "[MR]: Could not read dir for "
             << mapPath << " : " << strerror(errno) << endl;
      ids.clear();
      exit(1);
   } else {
      for( int i = 0; i < nbr; ++i ) {
         if ( StringUtility::endsWithStr( entries[i], ".mcm", true ) ) {
            // Is a map.
            // Remove ending .mcm
            int len = strlen(entries[i]);
            for ( int j = len - 1; j >= 0; --j ) {
               if ( entries[i][j] == '.' ) {
                  entries[i][j] = '\0';
                  ids.insert(strtoul(entries[i], NULL, 16) );
               }
            }
         }
         mc2dbg4 << "Entry : " << entries[i] << endl;
      }
   }
   Utility::deleteStrArray(entries, nbr);
} // getMapIDsInDir




MapGenUtil::mapSupAndVerPair
MapGenUtil::getMapSupAndVer( const char* gdfMapOrigin ){

   MapGenUtil::mapSupAndVerPair result;
   result.first = MapGenEnums::unknownSupplier;
   result.second = MapGenEnums::invMapVersion;

   if ( gdfMapOrigin == NULL ) {
      return result;
   }
   
   char* tmp = new char[strlen(gdfMapOrigin)+1];
   
   if ((gdfMapOrigin != NULL) && strlen(gdfMapOrigin) > 0) {
      // Check if the map has a maporigin
      // skip the gdf file name.
      
      const char* mapVersionStr = "";

      uint32 i = 0;
      bool found = false;
      while ((i < strlen(gdfMapOrigin)) && !found) {
         if (gdfMapOrigin[i] != '_')
            tmp[i] = gdfMapOrigin[i];
         else {
            found = true;
            tmp[i] = '\0';
            mapVersionStr = &(gdfMapOrigin[i+1]);
            // tmp[i] points at an '_', must
            // be followed by '\0', therfore
            // safe to increase i.
         }
         i++;
      }
      tmp[i] = '\0';

      // Found the supplier string, translate it to mapSuppliers type.
      mc2dbg8 << "msrc: " << tmp << endl;
      MapGenEnums::mapSupplier mapSup = getMapSupFromString( tmp );

      // Translate the map version string to map version type.
      mc2dbg8 << "mver: " << mapVersionStr << endl;
      MapGenEnums::mapVersion mapVer = 
         getMapVerFromString( mapSup, mapVersionStr );

      result.first = mapSup;
      result.second = mapVer;
   }
   
   delete[] tmp;
   return result;

} //  getMapSupAndVer


MapGenEnums::mapSupplier
MapGenUtil::getMapSupplier( const char* gdfMapOrigin ){
   MapGenEnums::mapSupplier result = 
      getMapSupAndVer( gdfMapOrigin ).first;
   return result;
} //  getMapSupplier

void
MapGenUtil::initMapSupMapping(){

   if (m_mapSupByMapSupName.size() == 0){
      // Only init if not inited before.

      // Fill in m_mapSupByMapSupName
      m_mapSupByMapSupName.insert(make_pair(MC2String("NavTech"),
                                            MapGenEnums::NavTech));
      m_mapSupByMapSupName.insert(make_pair(MC2String("TeleAtlas"),
                                            MapGenEnums::TeleAtlas));
      m_mapSupByMapSupName.insert(make_pair(MC2String("AND")
                                            ,MapGenEnums::AND));
      m_mapSupByMapSupName.insert(make_pair(MC2String("TopMap"),
                                            MapGenEnums::TopMap));
      m_mapSupByMapSupName.insert(make_pair(MC2String("GDT"),
                                            MapGenEnums::GDT));
      m_mapSupByMapSupName.insert(make_pair(MC2String("Infotech"),
                                            MapGenEnums::Infotech));
      m_mapSupByMapSupName.insert(make_pair(MC2String("Monolit"),
                                            MapGenEnums::Monolit));
      m_mapSupByMapSupName.insert(make_pair(MC2String("Spinfo"),
                                            MapGenEnums::Spinfo));
      m_mapSupByMapSupName.insert(make_pair(MC2String("CEInfoSystems"),
                                            MapGenEnums::CEInfoSystems));
      m_mapSupByMapSupName.insert(make_pair(MC2String("GISrael"),
                                            MapGenEnums::GISrael));
      m_mapSupByMapSupName.insert(make_pair(MC2String("DMapas"),
                                            MapGenEnums::DMapas));
      m_mapSupByMapSupName.insert(make_pair(MC2String("NavTurk"),
                                            MapGenEnums::NavTurk));
      m_mapSupByMapSupName.insert(make_pair(MC2String("Carmenta"),
                                            MapGenEnums::Carmenta));
      m_mapSupByMapSupName.insert(make_pair(MC2String("OpenStreetMap"),
                                            MapGenEnums::OpenStreetMap));

      // Fill in m_mapSupNameByMapSup
      m_mapSupNameByMapSup.clear();
      map<MC2String, MapGenEnums::mapSupplier>::const_iterator it = 
         m_mapSupByMapSupName.begin();
      while(it != m_mapSupByMapSupName.end()) {
         m_mapSupNameByMapSup.insert(make_pair(it->second,it->first));
         ++it;
      }
   }
}

MapGenEnums::mapSupplier
MapGenUtil::getMapSupFromString(  MC2String mapSupStr ){
   MapGenEnums::mapSupplier result = MapGenEnums::unknownSupplier;
   initMapSupMapping();

   map<MC2String, MapGenEnums::mapSupplier>::const_iterator it = 
      m_mapSupByMapSupName.find(mapSupStr);
   if ( it != m_mapSupByMapSupName.end() ){
      result = it->second;
   }
   else {
      mc2log << warn << "MapGenUtil::getMapSupFromString does not"
             << " handle map supplier string '" << mapSupStr << "'" << endl;
   }
   return result;
   
} // getMapSupFromString

MC2String
MapGenUtil::getIdStringFromMapSupplier (MapGenEnums::mapSupplier mapSup){
   MC2String result = "";
   initMapSupMapping();
   
   map<MapGenEnums::mapSupplier, MC2String>::const_iterator it = 
      m_mapSupNameByMapSup.find(mapSup);
   if ( it != m_mapSupNameByMapSup.end() ){
      result = it->second;
   }
   else {
      mc2log << warn << "MapGenUtil::getIdStringFromMapSupplier does not"
             << " handle map supplier ID '" << mapSup << "'" << endl;
   }
   return result;

} //getIdStringFromMapSupplier


MapGenEnums::mapVersion
MapGenUtil::getMapVerFromString(  MapGenEnums::mapSupplier mapSup,
                                  MC2String mapVerStr ){
   MapGenEnums::mapVersion result = MapGenEnums::invMapVersion;
   

   if ( mapSup == MapGenEnums::TeleAtlas ){

      // TeleAtlas

      if ( mapVerStr == "2002_2" ) {
         result = MapGenEnums::TA_2002_2;
      }
      else if ( mapVerStr == "2003_1"){
         result = MapGenEnums::TA_2003_1;
      }
      else if ( mapVerStr == "2003_2"){
         result = MapGenEnums::TA_2003_2;
      }
      else if ( mapVerStr == "2003_3"){
         result = MapGenEnums::TA_2003_3;
      }
      else if ( mapVerStr == "2004_1"){
         result = MapGenEnums::TA_2004_1;
      }
      else if ( mapVerStr == "2004_2"){
         result = MapGenEnums::TA_2004_2;
      }
      else if ( mapVerStr == "2005_1"){
         result = MapGenEnums::TA_2005_1;
      }
      else if ( mapVerStr == "2005_2"){
         result = MapGenEnums::TA_2005_2;
      }
      else if ( mapVerStr == "2005_3"){
         result = MapGenEnums::TA_2005_3;
      }
      else if ( mapVerStr == "2006_01"){
         result = MapGenEnums::TA_2006_01;
      }
      else if ( mapVerStr == "2006_02"){
         result = MapGenEnums::TA_2006_02;
      }
      else if ( mapVerStr == "2006_04"){
         result = MapGenEnums::TA_2006_04;
      }
      else if ( mapVerStr == "2006_07"){
         result = MapGenEnums::TA_2006_07;
      }
      else if ( mapVerStr == "2006_09"){
         result = MapGenEnums::TA_2006_09;
      }
      else if ( mapVerStr == "2006_10"){
         result = MapGenEnums::TA_2006_10;
      }
      else if ( mapVerStr == "2006_11"){
         result = MapGenEnums::TA_2006_11;
      }
      else if ( mapVerStr == "2007_01"){
         result = MapGenEnums::TA_2007_01;
      }
      else if ( mapVerStr == "2007_02"){
         result = MapGenEnums::TA_2007_02;
      }
      else if ( mapVerStr == "2007_04"){
         result = MapGenEnums::TA_2007_04;
      }
      else if ( mapVerStr == "2007_05"){
         result = MapGenEnums::TA_2007_05;
      }
      else if ( mapVerStr == "2007_07"){
         result = MapGenEnums::TA_2007_07;
      }
      else if ( mapVerStr == "2007_09"){
         result = MapGenEnums::TA_2007_09;
      }
      else if ( mapVerStr == "2007_10"){
         result = MapGenEnums::TA_2007_10;
      }
      else if ( mapVerStr == "2007_12"){
         result = MapGenEnums::TA_2007_12;
      }
      else if ( mapVerStr == "2008_01"){
         result = MapGenEnums::TA_2008_01;
      }
      else if ( mapVerStr == "2008_v1_31"){
         result = MapGenEnums::TA_2008_v1_31;
      }
      else if ( mapVerStr == "2008_02"){
         result = MapGenEnums::TA_2008_02;
      }
      else if ( mapVerStr == "2008_v8_0"){
         result = MapGenEnums::TA_2008_v8_0;
      }
      else if ( mapVerStr == "2008_03"){
         result = MapGenEnums::TA_2008_03;
      }
      else if ( mapVerStr == "2008_04"){
         result = MapGenEnums::TA_2008_04;
      }
      else if ( mapVerStr == "2008_06"){
         result = MapGenEnums::TA_2008_06;
      }
      else if ( mapVerStr == "2008_05"){
         result = MapGenEnums::TA_2008_05;
      }
      else if ( mapVerStr == "2008_07"){
         result = MapGenEnums::TA_2008_07;
      }
      else if ( mapVerStr == "2008_09"){
         result = MapGenEnums::TA_2008_09;
      }
      else if ( mapVerStr == "2008_10"){
         result = MapGenEnums::TA_2008_10;
      }
      else if ( mapVerStr == "2009_01"){
         result = MapGenEnums::TA_2009_01;
      }
      else if ( mapVerStr == "2009_02"){
         result = MapGenEnums::TA_2009_02;
      }
      else if ( mapVerStr == "2009_03"){
         result = MapGenEnums::TA_2009_03;
      }
      else if ( mapVerStr == "2009_06"){
         result = MapGenEnums::TA_2009_06;
      }
      else if ( mapVerStr == "2009_07"){
         result = MapGenEnums::TA_2009_07;
      }
      else if ( mapVerStr == "2009_08"){
         result = MapGenEnums::TA_2009_08;
      }
      else if ( mapVerStr == "2009_09"){
         result = MapGenEnums::TA_2009_09;
      }
      else if ( mapVerStr == "2009_10"){
         result = MapGenEnums::TA_2009_10;
      }
      else if ( mapVerStr == "2009_11"){
         result = MapGenEnums::TA_2009_11;
      }
      else if ( mapVerStr == "2009_12"){
         result = MapGenEnums::TA_2009_12;
      }
      else if ( mapVerStr == "2010_06"){
         result = MapGenEnums::TA_2010_06;
      }
   }
   else if ( mapSup == MapGenEnums::TopMap ){

      // TopMap
         
      if ( mapVerStr == "2004_08"){
         result = MapGenEnums::TM_2004_08;
      }
      else if ( mapVerStr == "2004_09"){
         result = MapGenEnums::TM_2004_09;
      }
      else if ( mapVerStr == "2004_12"){
         result = MapGenEnums::TM_2004_12;
      }
      else if ( mapVerStr == "2005_08"){
         result = MapGenEnums::TM_2005_08;
      }
      else if ( mapVerStr == "2006_2"){
         result = MapGenEnums::TM_2006_2;
      }
      else if ( mapVerStr == "2006_3"){
         result = MapGenEnums::TM_2006_3;
      }
      else if ( mapVerStr == "2006_4"){
         result = MapGenEnums::TM_2006_4;
      }
      else if ( mapVerStr == "2007_1"){
         result = MapGenEnums::TM_2007_1;
      }
      else if ( mapVerStr == "2007_2"){
         result = MapGenEnums::TM_2007_2;
      }
      else if ( mapVerStr == "2008_1"){
         result = MapGenEnums::TM_2008_1;
      }
   }
   else if ( mapSup == MapGenEnums::NavTech ){

      // NavTech

      //     AND GDF releases,
      //     extracted with NavTech GDF decoder.
      if ( mapVerStr == "2005_H1"){
         result = MapGenEnums::AND_2005_H1;
      }
      else if ( mapVerStr == "2005_H2"){
         result = MapGenEnums::AND_2005_H2;
      }
      else if ( mapVerStr == "2005_H2_lev6"){
         result = MapGenEnums::AND_2005_H2_lev6;
      }
      else if ( mapVerStr == "2006_H1"){
         result = MapGenEnums::AND_2006_H1;
      }
      else if ( mapVerStr == "2006_H1_lev2"){
         result = MapGenEnums::AND_2006_H1_lev2;
      }
      else if ( mapVerStr == "2006_H2_lev2"){
         result = MapGenEnums::AND_2006_H2_lev2;
      }


      // Real NavTech

      else if ( mapVerStr == "200607_smpl" ){
         result = MapGenEnums::NT_200607_smpl;
      }
      else if ( mapVerStr == "200712" ){
         result = MapGenEnums::NT_200712;
      }
      else if ( mapVerStr == "2007_Q2" ){
         result = MapGenEnums::NT_2007_Q2;
      }
      else if ( mapVerStr == "2007_Q4" ){
         result = MapGenEnums::NT_2007_Q4;
      }
      else if ( mapVerStr == "2008_Q3" ){
         result = MapGenEnums::NT_2008_Q3;
      }
   } 
   else if ( mapSup == MapGenEnums::AND ){

      // AND
      
      if ( mapVerStr == "2005_H1"){
         result = MapGenEnums::AND_2005_H1;
      }
      else if ( mapVerStr == "2005_H2"){
         result = MapGenEnums::AND_2005_H2;
      }
      else if ( mapVerStr == "2005_H2_lev6"){
         result = MapGenEnums::AND_2005_H2_lev6;
      }
      else if ( mapVerStr == "2006_H1"){
         result = MapGenEnums::AND_2006_H1;
      }
      else if ( mapVerStr == "2006_H1_lev2"){
         result = MapGenEnums::AND_2006_H1_lev2;
      }
      else if ( mapVerStr == "2006_H2_lev2"){
         result = MapGenEnums::AND_2006_H2_lev2;
      }
      else if ( mapVerStr == "2006_H2"){
         result = MapGenEnums::AND_2006_H2;
      }
   }
   else if ( mapSup == MapGenEnums::GDT ){

      // GDT
      
      if ( mapVerStr == "2005_05"){
         result = MapGenEnums::GDT_2005_05;
      }
   }
   else if ( mapSup == MapGenEnums::Infotech ){

      // Infotech
      
      if ( mapVerStr == "200602"){
         result = MapGenEnums::Infotech_200602;
      }
      else if ( mapVerStr == "200606"){
         result = MapGenEnums::Infotech_200606;
      }
   }
   else if ( mapSup == MapGenEnums::Monolit ){

      // Monolit
      
      if ( mapVerStr == "200603"){
         result = MapGenEnums::Monolit_200603;
      }
      else if ( mapVerStr == "200605"){
         result = MapGenEnums::Monolit_200605;
      }
      else if ( mapVerStr == "2007_02"){
         result = MapGenEnums::Monolit_2007_02;
      }
      else if ( mapVerStr == "200706"){
         result = MapGenEnums::Monolit_200706;
      }
      else if ( mapVerStr == "200801"){
         result = MapGenEnums::Monolit_200801;
      }
   }
   else if ( mapSup == MapGenEnums::CEInfoSystems ){

      // CEInfoSystems
      
      if ( mapVerStr == "200702"){
         result = MapGenEnums::CEInfoSystems_200702;
      }
      else if ( mapVerStr == "200704"){
         result = MapGenEnums::CEInfoSystems_200704;
      }
      else if ( mapVerStr == "200705"){
         result = MapGenEnums::CEInfoSystems_200705;
      }
      else if ( mapVerStr == "200705_2"){
         result = MapGenEnums::CEInfoSystems_200705_2;
      }
      else if ( mapVerStr == "200707"){
         result = MapGenEnums::CEInfoSystems_200707;
      }
      else if ( mapVerStr == "200710"){
         result = MapGenEnums::CEInfoSystems_200710;
      }
      else if ( mapVerStr == "200712"){
         result = MapGenEnums::CEInfoSystems_200712;
      }
      else if ( mapVerStr == "200805"){
         result = MapGenEnums::CEInfoSystems_200805;
      }
      else if ( mapVerStr == "200811"){
         result = MapGenEnums::CEInfoSystems_200811;
      }
      else if ( mapVerStr == "200905"){
         result = MapGenEnums::CEInfoSystems_200905;
      }
   }
   else if ( mapSup == MapGenEnums::GISrael ){

      // GISrael
      if ( mapVerStr == "200705"){
         result = MapGenEnums::GISrael_200705;
      }
      else if ( mapVerStr == "200706"){
         result = MapGenEnums::GISrael_200706;
      }
      else if ( mapVerStr == "200801"){
         result = MapGenEnums::GISrael_200801;
      }
   }
   else if ( mapSup == MapGenEnums::DMapas ){

      // DMapas
      if ( mapVerStr == "200708"){
         result = MapGenEnums::DMapas_200708;
      }
      else if ( mapVerStr == "200709"){
         result = MapGenEnums::DMapas_200709;
      }
      else if ( mapVerStr == "200710"){
         result = MapGenEnums::DMapas_200710;
      }
      else if ( mapVerStr == "200711"){
         result = MapGenEnums::DMapas_200711;
      }
      else if ( mapVerStr == "200802"){
         result = MapGenEnums::DMapas_200802;
      }
      else if ( mapVerStr == "200803"){
         result = MapGenEnums::DMapas_200803;
      }
      else if ( mapVerStr == "200805"){
         result = MapGenEnums::DMapas_200805;
      }
   }

   else if ( mapSup == MapGenEnums::NavTurk ){

      // NavTurk
      if ( mapVerStr == "200805"){
         result = MapGenEnums::NavTurk_200805;
      }
   }
   
   else if ( mapSup == MapGenEnums::Carmenta ){

      // Carmenta
      if ( mapVerStr == "200809"){
         result = MapGenEnums::Carmenta_200809;
      }
   }
   
   else if ( mapSup == MapGenEnums::OpenStreetMap ){

      // OpenStreetMap
      if ( mapVerStr == "201005"){
         result = MapGenEnums::OSM_201005;
      }
   }
   
   return result;
} //  getMapVerFromString

void
MapGenUtil::setFileName(OldGenericMap* theMap, const char* path)
{
   if ( theMap == NULL ){
      mc2log << error << "MapGenUtil::setFileName theMap==NULL" << endl;
      exit(1);
   }
   theMap->setFilename(path);
      
} // setFileName


bool
MapGenUtil::loadTxtFileTable( MC2String fileName, 
                              uint32 nbrCols,
                              MC2String colSeparator,
                              vector<vector<MC2String> >& tableContent,
                              uint32 skipStartRows /* =0 */,
                              uint32 skipEndRows /* =0 */,
                              map<MC2String, uint32>* prefixNbrCols, /*=NULL*/
                              set<uint32>* throwAwayCols /*NULL*/ )
{
   bool result = true;
   // Open the file.
   ifstream file( fileName.c_str(), ios::in );
   if ( ! file ) {
      mc2log << error << "MapGenUtil::loadTxtFileTable:"
             << " Could not open " << fileName << endl;
      exit(1);
   }

   // Read content of file row by row.
   vector<MC2String> rowStrings;

   uint32 tmpCharSize = 1024000;
   char* tmpChars = new char[tmpCharSize];

   // Read past the first skipStartRows rows.
   for (uint32 i = 0; i< skipStartRows; i++){
      file.getline(tmpChars, tmpCharSize);
      mc2dbg << "Skipping: " << tmpChars << endl;
   }

   file.getline(tmpChars, tmpCharSize);
   if ( tmpChars[strlen(tmpChars) - 1] == '\15' ){
      // Found a carriage return typical for windows text files, remove it.
      tmpChars[strlen(tmpChars) - 1] = '\0';
   }
   MC2String rowString = tmpChars;
   while ( ! file.eof() ){
      rowStrings.push_back(rowString);
      file.getline(tmpChars, tmpCharSize);
      if ( tmpChars[strlen(tmpChars) - 1] == '\15' ){
         // Found a carriage return typical for windows text files, 
         // remove it.
         tmpChars[strlen(tmpChars) - 1] = '\0';
      }
      rowString = tmpChars;
   }
   delete[] tmpChars;
   tmpChars = NULL;

   // Split each row into columns.
   for ( uint32 i=0; i<rowStrings.size() - skipEndRows; i++){
      uint32 nbrColsOfRow=0;
      vector<MC2String> row;
      MC2String leftOfRow = rowStrings[i];
      
      bool done = false;
   
      while (!done){
         if ( leftOfRow.length() == 0 ){
            done = true;
         }
   
         uint32 pos = leftOfRow.find( colSeparator );
         if ( pos == MAX_UINT32 ) {
            MC2String col = leftOfRow;
            row.push_back(col);
            nbrColsOfRow++;
            done = true;
         }
      
         if (!done){
            MC2String col = leftOfRow.substr(0, pos);
         row.push_back(col);
         nbrColsOfRow++;
         leftOfRow = leftOfRow.substr( pos + colSeparator.length(), 
                                       MC2String::npos);
         }
      }
      uint32 checkNbrCols = nbrCols;
      if ( ( prefixNbrCols != NULL) && 
           ( prefixNbrCols->find(row[0]) != prefixNbrCols->end() ) ){
         checkNbrCols = prefixNbrCols->find(row[0])->second;
      }
      if ( checkNbrCols != nbrColsOfRow ){
         mc2log << error << "MapGenUtil::loadTxtFileTable: "
                << "Wrong number of columns of row " << i << ", is "
                << nbrColsOfRow << " should have been " << nbrCols << endl;
         mc2dbg << "Row: " << rowStrings[i] << endl;
         result = false;
         exit(1);
      }

      // Throw away unwanted columns to save memory.
      if ( throwAwayCols != NULL ) {
         for ( set<uint32>::const_iterator colNbrIt = throwAwayCols->begin();
               colNbrIt != throwAwayCols->end(); ++colNbrIt ){
            row[*colNbrIt] = "";

         }
      }

      tableContent.push_back(row);
   }

   return result;

} // loadTxtFileTable

bool
MapGenUtil::saveTxtFileTable( MC2String fileName, 
                               uint32 nbrCols,
                               MC2String colSeparator,
                               vector<vector<MC2String> >& tableContent)
{
   bool result = true;

   // Open the file.
   ofstream file( fileName.c_str(), ios::out );
   if ( ! file ) {
      mc2log << error << "MapGenUtil::saveTxtFileTable:"
             << " Could not open " << fileName << endl;
      exit(1);
   }

   for (uint32 i=0; i<tableContent.size(); i++){
      if (tableContent[i].size() != nbrCols ){
         mc2log << error << "MapGenUtil::saveTxtFileTable: "
                << "Wrong number of columns of row " << i << ", is "
                << tableContent[i].size() << " should have been " 
                << nbrCols << endl;
         result = false;
         exit(1);
      }
      for (uint32 j=0; j<tableContent[i].size(); j++ ){
         file << tableContent[i][j];
         if ( j != tableContent[i].size() - 1 ){
            // Not the last column
            file << colSeparator;
         }
      }
      file << endl;
   }
   file.close();

   return result;

} // loadTxtFileTable

bool
MapGenUtil::fileExists(MC2String fileName){
   // Test to open file.
   ifstream testFile( fileName.c_str() );
   if (testFile.is_open()){
      testFile.close();
      return true;
   }
   else {
      return false;
   }
} //fileExists

StringTable::countryCode 
MapGenUtil::getCountryCodeFromGmsName( const MC2String& gmsName ){
   StringTable::countryCode countryCode = StringTable::NBR_COUNTRY_CODES;

   // Set the country code
   if (strcasecmp(gmsName.c_str(), "sweden") == 0) {
      countryCode = StringTable::SWEDEN_CC;
   } else if (strcasecmp(gmsName.c_str(), "norway") == 0) {
      countryCode = StringTable::NORWAY_CC;
   } else if (strcasecmp(gmsName.c_str(), "denmark") == 0) {
      countryCode = StringTable::DENMARK_CC;
   } else if (strcasecmp(gmsName.c_str(), "finland") == 0) {
      countryCode = StringTable::FINLAND_CC;
   } else if (strcasecmp(gmsName.c_str(), "germany") == 0) {
      countryCode = StringTable::GERMANY_CC; 
   } else if (strcasecmp(gmsName.c_str(), "england") == 0) {
      countryCode = StringTable::ENGLAND_CC;
   } else if (strcasecmp(gmsName.c_str(), "belgium") == 0) {
      countryCode = StringTable::BELGIUM_CC;
   } else if (strcasecmp(gmsName.c_str(), "netherlands") == 0) {
      countryCode = StringTable::NETHERLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "luxembourg") == 0) {
      countryCode = StringTable::LUXEMBOURG_CC;
   } else if (strcasecmp(gmsName.c_str(), "usa") == 0) {
      countryCode = StringTable::USA_CC;
   } else if (strcasecmp(gmsName.c_str(), "switzerland") == 0) {
      countryCode = StringTable::SWITZERLAND_CC;
   } else if (strcasecmp(gmsName.c_str(), "austria") == 0) {
      countryCode = StringTable::AUSTRIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "france") == 0) {
      countryCode = StringTable::FRANCE_CC;
   } else if (strcasecmp(gmsName.c_str(), "spain") == 0) {
      countryCode = StringTable::SPAIN_CC;
   } else if (strcasecmp(gmsName.c_str(), "andorra") == 0) {
      countryCode = StringTable::ANDORRA_CC;
   } else if (strcasecmp(gmsName.c_str(), "liechtenstein") == 0) {
      countryCode = StringTable::LIECHTENSTEIN_CC;
   } else if (strcasecmp(gmsName.c_str(), "italy") == 0) {
      countryCode = StringTable::ITALY_CC;
   } else if (strcasecmp(gmsName.c_str(), "monaco") == 0) {
      countryCode = StringTable::MONACO_CC;
   } else if (strcasecmp(gmsName.c_str(), "ireland") == 0) {
      countryCode = StringTable::IRELAND_CC;
   } else if (strcasecmp(gmsName.c_str(), "portugal") == 0) {
      countryCode = StringTable::PORTUGAL_CC;
   } else if (strcasecmp(gmsName.c_str(), "canada") == 0) {
      countryCode = StringTable::CANADA_CC;
   } else if (strcasecmp(gmsName.c_str(), "hungary") == 0) {
      countryCode = StringTable::HUNGARY_CC;
   } else if (strcasecmp(gmsName.c_str(), "czech_republic") == 0) {
      countryCode = StringTable::CZECH_REPUBLIC_CC;
   } else if (strcasecmp(gmsName.c_str(), "poland") == 0) {
      countryCode = StringTable::POLAND_CC;
   } else if (strcasecmp(gmsName.c_str(), "greece") == 0) {
      countryCode = StringTable::GREECE_CC;
   } else if (strcasecmp(gmsName.c_str(), "israel") == 0) {
      countryCode = StringTable::ISRAEL_CC;
   } else if (strcasecmp(gmsName.c_str(), "brazil") == 0) {
      countryCode = StringTable::BRAZIL_CC;
   } else if (strcasecmp(gmsName.c_str(), "slovakia") == 0) {
      countryCode = StringTable::SLOVAKIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "russia") == 0) {
      countryCode = StringTable::RUSSIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "turkey") == 0) {
      countryCode = StringTable::TURKEY_CC;
   } else if (strcasecmp(gmsName.c_str(), "hong_kong") == 0) {
      countryCode = StringTable::HONG_KONG_CC;
   } else if (strcasecmp(gmsName.c_str(), "singapore") == 0) {
      countryCode = StringTable::SINGAPORE_CC;
   } else if (strcasecmp(gmsName.c_str(), "croatia") == 0) {
      countryCode = StringTable::CROATIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "slovenia") == 0) {
      countryCode = StringTable::SLOVENIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "australia") == 0) {
      countryCode = StringTable::AUSTRALIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "united_arab_emirates") == 0) {
      countryCode = StringTable::UAE_CC;
   } else if (strcasecmp(gmsName.c_str(), "bahrain") == 0) {
      countryCode = StringTable::BAHRAIN_CC;
   } else if (strcasecmp(gmsName.c_str(), "afghanistan") == 0) {
      countryCode = StringTable::AFGHANISTAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "albania") == 0) {
      countryCode = StringTable::ALBANIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "algeria") == 0) {
      countryCode = StringTable::ALGERIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "american_samoa") == 0) {
      countryCode = StringTable::AMERICAN_SAMOA_CC;
   } else if (strcasecmp(gmsName.c_str(), "angola") == 0) {
      countryCode = StringTable::ANGOLA_CC;
   } else if (strcasecmp(gmsName.c_str(), "anguilla") == 0) {
      countryCode = StringTable::ANGUILLA_CC;
   } else if (strcasecmp(gmsName.c_str(), "antarctica") == 0) {
      countryCode = StringTable::ANTARCTICA_CC;
   } else if (strcasecmp(gmsName.c_str(), "antigua_and_barbuda") == 0) {
      countryCode = StringTable::ANTIGUA_AND_BARBUDA_CC;
   } else if (strcasecmp(gmsName.c_str(), "argentina") == 0) {
      countryCode = StringTable::ARGENTINA_CC;
   } else if (strcasecmp(gmsName.c_str(), "armenia") == 0) {
      countryCode = StringTable::ARMENIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "aruba") == 0) {
      countryCode = StringTable::ARUBA_CC;
   } else if (strcasecmp(gmsName.c_str(), "azerbaijan") == 0) {
      countryCode = StringTable::AZERBAIJAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "bahamas") == 0) {
      countryCode = StringTable::BAHAMAS_CC;
   } else if (strcasecmp(gmsName.c_str(), "bangladesh") == 0) {
      countryCode = StringTable::BANGLADESH_CC;
   } else if (strcasecmp(gmsName.c_str(), "barbados") == 0) {
      countryCode = StringTable::BARBADOS_CC;
   } else if (strcasecmp(gmsName.c_str(), "belarus") == 0) {
      countryCode = StringTable::BELARUS_CC;
   } else if (strcasecmp(gmsName.c_str(), "belize") == 0) {
      countryCode = StringTable::BELIZE_CC;
   } else if (strcasecmp(gmsName.c_str(), "benin") == 0) {
      countryCode = StringTable::BENIN_CC;
   } else if (strcasecmp(gmsName.c_str(), "bermuda") == 0) {
      countryCode = StringTable::BERMUDA_CC;
   } else if (strcasecmp(gmsName.c_str(), "bhutan") == 0) {
      countryCode = StringTable::BHUTAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "bolivia") == 0) {
      countryCode = StringTable::BOLIVIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "bosnia") == 0) {
      countryCode = StringTable::BOSNIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "botswana") == 0) {
      countryCode = StringTable::BOTSWANA_CC;
   } else if (strcasecmp(gmsName.c_str(), "british_virgin_islands") == 0) {
      countryCode = StringTable::BRITISH_VIRGIN_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "brunei_darussalam") == 0) {
      countryCode = StringTable::BRUNEI_DARUSSALAM_CC;
   } else if (strcasecmp(gmsName.c_str(), "bulgaria") == 0) {
      countryCode = StringTable::BULGARIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "burkina_faso") == 0) {
      countryCode = StringTable::BURKINA_FASO_CC;
   } else if (strcasecmp(gmsName.c_str(), "burundi") == 0) {
      countryCode = StringTable::BURUNDI_CC;
   } else if (strcasecmp(gmsName.c_str(), "cambodia") == 0) {
      countryCode = StringTable::CAMBODIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "cameroon") == 0) {
      countryCode = StringTable::CAMEROON_CC;
   } else if (strcasecmp(gmsName.c_str(), "cape_verde") == 0) {
      countryCode = StringTable::CAPE_VERDE_CC;
   } else if (strcasecmp(gmsName.c_str(), "cayman_islands") == 0) {
      countryCode = StringTable::CAYMAN_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "central_african_republic") == 0) {
      countryCode = StringTable::CENTRAL_AFRICAN_REPUBLIC_CC;
   } else if (strcasecmp(gmsName.c_str(), "chad") == 0) {
      countryCode = StringTable::CHAD_CC;
   } else if (strcasecmp(gmsName.c_str(), "chile") == 0) {
      countryCode = StringTable::CHILE_CC;
   } else if (strcasecmp(gmsName.c_str(), "china") == 0) {
      countryCode = StringTable::CHINA_CC;
   } else if (strcasecmp(gmsName.c_str(), "colombia") == 0) {
      countryCode = StringTable::COLOMBIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "comoros") == 0) {
      countryCode = StringTable::COMOROS_CC;
   } else if (strcasecmp(gmsName.c_str(), "congo") == 0) {
      countryCode = StringTable::CONGO_CC;
   } else if (strcasecmp(gmsName.c_str(), "cook_islands") == 0) {
      countryCode = StringTable::COOK_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "costa_rica") == 0) {
      countryCode = StringTable::COSTA_RICA_CC;
   } else if (strcasecmp(gmsName.c_str(), "cuba") == 0) {
      countryCode = StringTable::CUBA_CC;
   } else if (strcasecmp(gmsName.c_str(), "cyprus") == 0) {
      countryCode = StringTable::CYPRUS_CC;
   } else if (strcasecmp(gmsName.c_str(), "djibouti") == 0) {
      countryCode = StringTable::DJIBOUTI_CC;
   } else if (strcasecmp(gmsName.c_str(), "dominica") == 0) {
      countryCode = StringTable::DOMINICA_CC;
   } else if (strcasecmp(gmsName.c_str(), "dominican_republic") == 0) {
      countryCode = StringTable::DOMINICAN_REPUBLIC_CC;
   } else if (strcasecmp(gmsName.c_str(), "dr_congo") == 0) {
      countryCode = StringTable::DR_CONGO_CC;
   } else if (strcasecmp(gmsName.c_str(), "ecuador") == 0) {
      countryCode = StringTable::ECUADOR_CC;
   } else if (strcasecmp(gmsName.c_str(), "egypt") == 0) {
      countryCode = StringTable::EGYPT_CC;
   } else if (strcasecmp(gmsName.c_str(), "el_salvador") == 0) {
      countryCode = StringTable::EL_SALVADOR_CC;
   } else if (strcasecmp(gmsName.c_str(), "equatorial_guinea") == 0) {
      countryCode = StringTable::EQUATORIAL_GUINEA_CC;
   } else if (strcasecmp(gmsName.c_str(), "eritrea") == 0) {
      countryCode = StringTable::ERITREA_CC;
   } else if (strcasecmp(gmsName.c_str(), "estonia") == 0) {
      countryCode = StringTable::ESTONIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "ethiopia") == 0) {
      countryCode = StringTable::ETHIOPIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "faeroe_islands") == 0) {
      countryCode = StringTable::FAEROE_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "falkland_islands") == 0) {
      countryCode = StringTable::FALKLAND_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "fiji") == 0) {
      countryCode = StringTable::FIJI_CC;
   } else if (strcasecmp(gmsName.c_str(), "french_guiana") == 0) {
      countryCode = StringTable::FRENCH_GUIANA_CC;
   } else if (strcasecmp(gmsName.c_str(), "french_polynesia") == 0) {
      countryCode = StringTable::FRENCH_POLYNESIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "gabon") == 0) {
      countryCode = StringTable::GABON_CC;
   } else if (strcasecmp(gmsName.c_str(), "gambia") == 0) {
      countryCode = StringTable::GAMBIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "georgia_country") == 0) {
      countryCode = StringTable::GEORGIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "ghana") == 0) {
      countryCode = StringTable::GHANA_CC;
   } else if (strcasecmp(gmsName.c_str(), "greenland") == 0) {
      countryCode = StringTable::GREENLAND_CC;
   } else if (strcasecmp(gmsName.c_str(), "grenada") == 0) {
      countryCode = StringTable::GRENADA_CC;
   } else if (strcasecmp(gmsName.c_str(), "guadeloupe") == 0) {
      countryCode = StringTable::GUADELOUPE_CC;
   } else if (strcasecmp(gmsName.c_str(), "guam") == 0) {
      countryCode = StringTable::GUAM_CC;
   } else if (strcasecmp(gmsName.c_str(), "guatemala") == 0) {
      countryCode = StringTable::GUATEMALA_CC;
   } else if (strcasecmp(gmsName.c_str(), "guinea") == 0) {
      countryCode = StringTable::GUINEA_CC;
   } else if (strcasecmp(gmsName.c_str(), "guinea_bissau") == 0) {
      countryCode = StringTable::GUINEA_BISSAU_CC;
   } else if (strcasecmp(gmsName.c_str(), "guyana") == 0) {
      countryCode = StringTable::GUYANA_CC;
   } else if (strcasecmp(gmsName.c_str(), "haiti") == 0) {
      countryCode = StringTable::HAITI_CC;
   } else if (strcasecmp(gmsName.c_str(), "honduras") == 0) {
      countryCode = StringTable::HONDURAS_CC;
   } else if (strcasecmp(gmsName.c_str(), "iceland") == 0) {
      countryCode = StringTable::ICELAND_CC;
   } else if (strcasecmp(gmsName.c_str(), "india") == 0) {
      countryCode = StringTable::INDIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "indonesia") == 0) {
      countryCode = StringTable::INDONESIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "iran") == 0) {
      countryCode = StringTable::IRAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "iraq") == 0) {
      countryCode = StringTable::IRAQ_CC;
   } else if (strcasecmp(gmsName.c_str(), "ivory_coast") == 0) {
      countryCode = StringTable::IVORY_COAST_CC;
   } else if (strcasecmp(gmsName.c_str(), "jamaica") == 0) {
      countryCode = StringTable::JAMAICA_CC;
   } else if (strcasecmp(gmsName.c_str(), "japan") == 0) {
      countryCode = StringTable::JAPAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "jordan") == 0) {
      countryCode = StringTable::JORDAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "kazakhstan") == 0) {
      countryCode = StringTable::KAZAKHSTAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "kenya") == 0) {
      countryCode = StringTable::KENYA_CC;
   } else if (strcasecmp(gmsName.c_str(), "kiribati") == 0) {
      countryCode = StringTable::KIRIBATI_CC;
   } else if (strcasecmp(gmsName.c_str(), "kuwait") == 0) {
      countryCode = StringTable::KUWAIT_CC;
   } else if (strcasecmp(gmsName.c_str(), "kyrgyzstan") == 0) {
      countryCode = StringTable::KYRGYZSTAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "laos") == 0) {
      countryCode = StringTable::LAOS_CC;
   } else if (strcasecmp(gmsName.c_str(), "latvia") == 0) {
      countryCode = StringTable::LATVIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "lebanon") == 0) {
      countryCode = StringTable::LEBANON_CC;
   } else if (strcasecmp(gmsName.c_str(), "lesotho") == 0) {
      countryCode = StringTable::LESOTHO_CC;
   } else if (strcasecmp(gmsName.c_str(), "liberia") == 0) {
      countryCode = StringTable::LIBERIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "libya") == 0) {
      countryCode = StringTable::LIBYA_CC;
   } else if (strcasecmp(gmsName.c_str(), "lithuania") == 0) {
      countryCode = StringTable::LITHUANIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "macao") == 0) {
      countryCode = StringTable::MACAO_CC;
   } else if (strcasecmp(gmsName.c_str(), "macedonia") == 0) {
      countryCode = StringTable::MACEDONIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "madagascar") == 0) {
      countryCode = StringTable::MADAGASCAR_CC;
   } else if (strcasecmp(gmsName.c_str(), "malawi") == 0) {
      countryCode = StringTable::MALAWI_CC;
   } else if (strcasecmp(gmsName.c_str(), "malaysia") == 0) {
      countryCode = StringTable::MALAYSIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "maldives") == 0) {
      countryCode = StringTable::MALDIVES_CC;
   } else if (strcasecmp(gmsName.c_str(), "mali") == 0) {
      countryCode = StringTable::MALI_CC;
   } else if (strcasecmp(gmsName.c_str(), "malta") == 0) {
      countryCode = StringTable::MALTA_CC;
   } else if (strcasecmp(gmsName.c_str(), "marshall_islands") == 0) {
      countryCode = StringTable::MARSHALL_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "martinique") == 0) {
      countryCode = StringTable::MARTINIQUE_CC;
   } else if (strcasecmp(gmsName.c_str(), "mauritania") == 0) {
      countryCode = StringTable::MAURITANIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "mauritius") == 0) {
      countryCode = StringTable::MAURITIUS_CC;
   } else if (strcasecmp(gmsName.c_str(), "mayotte") == 0) {
      countryCode = StringTable::MAYOTTE_CC;
   } else if (strcasecmp(gmsName.c_str(), "mexico") == 0) {
      countryCode = StringTable::MEXICO_CC;
   } else if (strcasecmp(gmsName.c_str(), "micronesia") == 0) {
      countryCode = StringTable::MICRONESIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "moldova") == 0) {
      countryCode = StringTable::MOLDOVA_CC;
   } else if (strcasecmp(gmsName.c_str(), "mongolia") == 0) {
      countryCode = StringTable::MONGOLIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "montserrat") == 0) {
      countryCode = StringTable::MONTSERRAT_CC;
   } else if (strcasecmp(gmsName.c_str(), "morocco") == 0) {
      countryCode = StringTable::MOROCCO_CC;
   } else if (strcasecmp(gmsName.c_str(), "mozambique") == 0) {
      countryCode = StringTable::MOZAMBIQUE_CC;
   } else if (strcasecmp(gmsName.c_str(), "myanmar") == 0) {
      countryCode = StringTable::MYANMAR_CC;
   } else if (strcasecmp(gmsName.c_str(), "namibia") == 0) {
      countryCode = StringTable::NAMIBIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "nauru") == 0) {
      countryCode = StringTable::NAURU_CC;
   } else if (strcasecmp(gmsName.c_str(), "nepal") == 0) {
      countryCode = StringTable::NEPAL_CC;
   } else if (strcasecmp(gmsName.c_str(), "netherlands_antilles") == 0) {
      countryCode = StringTable::NETHERLANDS_ANTILLES_CC;
   } else if (strcasecmp(gmsName.c_str(), "new_caledonia") == 0) {
      countryCode = StringTable::NEW_CALEDONIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "new_zealand") == 0) {
      countryCode = StringTable::NEW_ZEALAND_CC;
   } else if (strcasecmp(gmsName.c_str(), "nicaragua") == 0) {
      countryCode = StringTable::NICARAGUA_CC;
   } else if (strcasecmp(gmsName.c_str(), "niger") == 0) {
      countryCode = StringTable::NIGER_CC;
   } else if (strcasecmp(gmsName.c_str(), "nigeria") == 0) {
      countryCode = StringTable::NIGERIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "niue") == 0) {
      countryCode = StringTable::NIUE_CC;
   } else if (strcasecmp(gmsName.c_str(), "northern_mariana_islands") == 0) {
      countryCode = StringTable::NORTHERN_MARIANA_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "north_korea") == 0) {
      countryCode = StringTable::NORTH_KOREA_CC;
   } else if (strcasecmp(gmsName.c_str(), "occupied_palestinian_territory") == 0) {
      countryCode = StringTable::OCCUPIED_PALESTINIAN_TERRITORY_CC;
   } else if (strcasecmp(gmsName.c_str(), "oman") == 0) {
      countryCode = StringTable::OMAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "pakistan") == 0) {
      countryCode = StringTable::PAKISTAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "palau") == 0) {
      countryCode = StringTable::PALAU_CC;
   } else if (strcasecmp(gmsName.c_str(), "panama") == 0) {
      countryCode = StringTable::PANAMA_CC;
   } else if (strcasecmp(gmsName.c_str(), "papua_new_guinea") == 0) {
      countryCode = StringTable::PAPUA_NEW_GUINEA_CC;
   } else if (strcasecmp(gmsName.c_str(), "paraguay") == 0) {
      countryCode = StringTable::PARAGUAY_CC;
   } else if (strcasecmp(gmsName.c_str(), "peru") == 0) {
      countryCode = StringTable::PERU_CC;
   } else if (strcasecmp(gmsName.c_str(), "philippines") == 0) {
      countryCode = StringTable::PHILIPPINES_CC;
   } else if (strcasecmp(gmsName.c_str(), "pitcairn") == 0) {
      countryCode = StringTable::PITCAIRN_CC;
   } else if (strcasecmp(gmsName.c_str(), "qatar") == 0) {
      countryCode = StringTable::QATAR_CC;
   } else if (strcasecmp(gmsName.c_str(), "reunion") == 0) {
      countryCode = StringTable::REUNION_CC;
   } else if (strcasecmp(gmsName.c_str(), "romania") == 0) {
      countryCode = StringTable::ROMANIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "rwanda") == 0) {
      countryCode = StringTable::RWANDA_CC;
   } else if (strcasecmp(gmsName.c_str(), "saint_helena") == 0) {
      countryCode = StringTable::SAINT_HELENA_CC;
   } else if (strcasecmp(gmsName.c_str(), "saint_kitts_and_nevis") == 0) {
      countryCode = StringTable::SAINT_KITTS_AND_NEVIS_CC;
   } else if (strcasecmp(gmsName.c_str(), "saint_lucia") == 0) {
      countryCode = StringTable::SAINT_LUCIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "saint_pierre_and_miquelon") == 0) {
      countryCode = StringTable::SAINT_PIERRE_AND_MIQUELON_CC;
   } else if (strcasecmp(gmsName.c_str(), "saint_vincent_and_the_grenadines") == 0) {
      countryCode = StringTable::SAINT_VINCENT_AND_THE_GRENADINES_CC;
   } else if (strcasecmp(gmsName.c_str(), "samoa") == 0) {
      countryCode = StringTable::SAMOA_CC;
   } else if (strcasecmp(gmsName.c_str(), "san_marino") == 0) {
      countryCode = StringTable::SAN_MARINO_CC;
   } else if (strcasecmp(gmsName.c_str(), "sao_tome_and_principe") == 0) {
      countryCode = StringTable::SAO_TOME_AND_PRINCIPE_CC;
   } else if (strcasecmp(gmsName.c_str(), "saudi_arabia") == 0) {
      countryCode = StringTable::SAUDI_ARABIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "senegal") == 0) {
      countryCode = StringTable::SENEGAL_CC;
   } else if (strcasecmp(gmsName.c_str(), "serbia_montenegro") == 0) {
      countryCode = StringTable::SERBIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "seychelles") == 0) {
      countryCode = StringTable::SEYCHELLES_CC;
   } else if (strcasecmp(gmsName.c_str(), "sierra_leone") == 0) {
      countryCode = StringTable::SIERRA_LEONE_CC;
   } else if (strcasecmp(gmsName.c_str(), "solomon_islands") == 0) {
      countryCode = StringTable::SOLOMON_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "somalia") == 0) {
      countryCode = StringTable::SOMALIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "south_africa") == 0) {
      countryCode = StringTable::SOUTH_AFRICA_CC;
   } else if (strcasecmp(gmsName.c_str(), "south_korea") == 0) {
      countryCode = StringTable::SOUTH_KOREA_CC;
   } else if (strcasecmp(gmsName.c_str(), "sri_lanka") == 0) {
      countryCode = StringTable::SRI_LANKA_CC;
   } else if (strcasecmp(gmsName.c_str(), "sudan") == 0) {
      countryCode = StringTable::SUDAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "suriname") == 0) {
      countryCode = StringTable::SURINAME_CC;
   } else if (strcasecmp(gmsName.c_str(), "svalbard_and_jan_mayen") == 0) {
      countryCode = StringTable::SVALBARD_AND_JAN_MAYEN_CC;
   } else if (strcasecmp(gmsName.c_str(), "swaziland") == 0) {
      countryCode = StringTable::SWAZILAND_CC;
   } else if (strcasecmp(gmsName.c_str(), "syria") == 0) {
      countryCode = StringTable::SYRIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "taiwan") == 0) {
      countryCode = StringTable::TAIWAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "tajikistan") == 0) {
      countryCode = StringTable::TAJIKISTAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "tanzania") == 0) {
      countryCode = StringTable::TANZANIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "thailand") == 0) {
      countryCode = StringTable::THAILAND_CC;
   } else if (strcasecmp(gmsName.c_str(), "timor_leste") == 0) {
      countryCode = StringTable::TIMOR_LESTE_CC;
   } else if (strcasecmp(gmsName.c_str(), "togo") == 0) {
      countryCode = StringTable::TOGO_CC;
   } else if (strcasecmp(gmsName.c_str(), "tokelau") == 0) {
      countryCode = StringTable::TOKELAU_CC;
   } else if (strcasecmp(gmsName.c_str(), "tonga") == 0) {
      countryCode = StringTable::TONGA_CC;
   } else if (strcasecmp(gmsName.c_str(), "trinidad_and_tobago") == 0) {
      countryCode = StringTable::TRINIDAD_AND_TOBAGO_CC;
   } else if (strcasecmp(gmsName.c_str(), "tunisia") == 0) {
      countryCode = StringTable::TUNISIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "turkmenistan") == 0) {
      countryCode = StringTable::TURKMENISTAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "turks_and_caicos_islands") == 0) {
      countryCode = StringTable::TURKS_AND_CAICOS_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "tuvalu") == 0) {
      countryCode = StringTable::TUVALU_CC;
   } else if (strcasecmp(gmsName.c_str(), "uganda") == 0) {
      countryCode = StringTable::UGANDA_CC;
   } else if (strcasecmp(gmsName.c_str(), "ukraine") == 0) {
      countryCode = StringTable::UKRAINE_CC;
   } else if (strcasecmp(gmsName.c_str(), "united_states_minor_outlying_islands") == 0) {
      countryCode = StringTable::UNITED_STATES_MINOR_OUTLYING_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "united_states_virgin_islands") == 0) {
      countryCode = StringTable::UNITED_STATES_VIRGIN_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "uruguay") == 0) {
      countryCode = StringTable::URUGUAY_CC;
   } else if (strcasecmp(gmsName.c_str(), "uzbekistan") == 0) {
      countryCode = StringTable::UZBEKISTAN_CC;
   } else if (strcasecmp(gmsName.c_str(), "vanuatu") == 0) {
      countryCode = StringTable::VANUATU_CC;
   } else if (strcasecmp(gmsName.c_str(), "venezuela") == 0) {
      countryCode = StringTable::VENEZUELA_CC;
   } else if (strcasecmp(gmsName.c_str(), "vietnam") == 0) {
      countryCode = StringTable::VIETNAM_CC;
   } else if (strcasecmp(gmsName.c_str(), "wallis_and_futuna_islands") == 0) {
      countryCode = StringTable::WALLIS_AND_FUTUNA_ISLANDS_CC;
   } else if (strcasecmp(gmsName.c_str(), "western_sahara") == 0) {
      countryCode = StringTable::WESTERN_SAHARA_CC;
   } else if (strcasecmp(gmsName.c_str(), "yemen") == 0) {
      countryCode = StringTable::YEMEN_CC;
   } else if (strcasecmp(gmsName.c_str(), "zambia") == 0) {
      countryCode = StringTable::ZAMBIA_CC;
   } else if (strcasecmp(gmsName.c_str(), "zimbabwe") == 0) {
      countryCode = StringTable::ZIMBABWE_CC;
   } else {
      countryCode = StringTable::NBR_COUNTRY_CODES;
   }
   return countryCode;

} // getCountryCodeFromGmsName


vector<uint32> 
MapGenUtil::getAllUndMapIDs(const OldOverviewMap* ovrMap)
{
   uint32 ovrMapID = ovrMap->getMapID();
   MC2_ASSERT(ovrMapID);
   
   vector<uint32> undIDs;
   uint32 coOvrID = ovrMapID+1;
   OldCountryOverviewMap* coOvrMap = dynamic_cast<OldCountryOverviewMap*>
      (OldGenericMap::createMap(coOvrID, ovrMap->getPathName()));
   mc2dbg << "ID: " << coOvrID << " path: " << ovrMap->getPathName() << endl;
   MC2_ASSERT(coOvrMap != NULL);

   for (uint32 i=0; i<coOvrMap->getNbrMaps(); i++){
      uint32 creationTime; // dummies
      int32 maxLat, minLon, minLat, maxLon; // dummies
      uint32 undMapID = coOvrMap->getMapData( i, creationTime, 
                                              maxLat, minLon, 
                                              minLat, maxLon);
      undIDs.push_back(undMapID);
   }
   delete coOvrMap;
   return undIDs;

} // getAllUndMapIDs

MC2String 
MapGenUtil::handleItemNameCase(const MC2String& name)
{
   return StringUtility::makeFirstInWordCapital(name, 
                                                true // Make roman upper
                                                );
} //handleItemNameCase

MC2String
MapGenUtil::intToBitFieldStr(uint32 bitField, uint32 nbrBits){
   MC2String result;
   for (int32 i=nbrBits-1; i>=0; i--){
      if ( bitField & (1 << i) ){
         result+='1';
      }
      else {
         result+='0';  
      }
   }
   return result;
} // intToBitFieldStr

