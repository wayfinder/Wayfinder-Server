/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "OldMapIdByName.h"


#include "MapGenUtil.h"
#include "OldGenericMapHeader.h"
#include "MapBits.h"

#include <fstream>

OldMapIdByName::OldMapIdByName( MC2String fileName ){
   m_fileName = fileName;
}

OldMapIdByName::mapIdAndHdlByName_t& 
OldMapIdByName::getWriteableMapIdByAndHdlName()
{
   return m_mapIdAndHdlByName;

} // getWriteableMapIdByAndHdlName


bool 
OldMapIdByName::initAndWriteFile(MC2String mapPath)
{
   // Map mapping map id to map name and handled status.
   // Setting all handled statuses to false.
   m_mapIdAndHdlByName.clear();
   uint32 mapID = 0;
   bool cont = true;
   while (cont) {

      OldGenericMapHeader* curMap = 
         new OldGenericMapHeader( mapID, mapPath.c_str() );
      if ( curMap->load() ){
         
         pair< MC2String, pair<uint32, bool> > nameToIdAndHdl;
         MC2String underviewName=curMap->getMapName();
         nameToIdAndHdl.first = curMap->getMapName();
         nameToIdAndHdl.second.first = mapID;
         nameToIdAndHdl.second.second = false; // handled.
         mc2log << info << "Read name of map: " << mapID 
                << ":" << underviewName << endl;
         
         m_mapIdAndHdlByName.insert( nameToIdAndHdl );
         
         delete curMap;
         curMap = NULL;
         mapID = MapBits::nextMapID(mapID);
      }
      else if ( MapBits::isUnderviewMap(mapID) ){
         // Continue with ovierview maps.
         mapID = FIRST_OVERVIEWMAP_ID; // defined in config.h
      }
      else if ( ( MapBits::isOverviewMap(mapID) ) && 
                (mapID < FIRST_SUPEROVERVIEWMAP_ID) ){
         // Continue with country overivew maps.
         mapID = FIRST_COUNTRYMAP_ID; // defined in config.h  
      }
      else if ( MapBits::isCountryMap(mapID) ){
         // Continue with super ovierview maps.
         mapID = FIRST_SUPEROVERVIEWMAP_ID; // defined in config.h  
      }
      else {
         // Have read all underview, overivew, country overiview and super
         // overviews.
         cont = false;
      }
   }
   mc2log << info << "OldMapIdByName::initAndWriteFile" 
          << "Done reading underview map names. Saving list in file" 
          << endl;

   // Write 
   bool result = OldMapIdByName::writeFile();
   return result;
} // initAndWriteFile


bool 
OldMapIdByName::fileExists()
{
   // Test to open file.
   mc2dbg << "Test to open file:" << m_fileName << endl;
   bool fileExists = MapGenUtil::fileExists(m_fileName);
   if ( fileExists ){
      mc2dbg << m_fileName << " exists." << endl;
   }
   else {
      mc2dbg << m_fileName << " does not exist." << endl;  
   }
   return fileExists;

} //  fileExists


   
bool
OldMapIdByName::writeFile() 
{
   /* Structure of the file pointed at by m_fileName
    * mapName mapID mapHdl
    *
    * mapName Name of underview map.
    * mapID   Map ID of underview map.
    * mapHdl  A string "TRUE" or "FALSE". Tells whether this map
    *         has been added to an overview map.
    */

   ofstream file( m_fileName.c_str(), ios::out|ios::trunc );
   if ( ! file ) {
      mc2log << error << "OldMapIdByName::writeFile. Could not open " 
             << m_fileName << endl;
      return false;
   }

   mapIdAndHdlByName_t::const_iterator it = m_mapIdAndHdlByName.begin();
   while ( it != m_mapIdAndHdlByName.end() ){
      MC2String mapName = it->first;
      uint32 mapID = it->second.first;
      MC2String mapHdl = ( it->second.second ? "TRUE": "FALSE");
      file << mapName << " " << mapID << " " << mapHdl << endl;
      ++it;
   }
   
   file.close();
   return true;

} // writeFile


bool
OldMapIdByName::readFile()
{
   // For documentation of the structure in this file, see definition of
   // writeFile.

   ifstream file( m_fileName.c_str(), ios::in );
   if ( ! file ) {
      mc2log << error << "OldMapIdByName::readFile. Could not open " 
             << m_fileName << endl;
      return false;
   }

   while ( ! file.eof() ) {
      pair<MC2String, pair<uint32, bool> > nameToIdAndHdl;
      MC2String mapHdlStr;

      file >> nameToIdAndHdl.first;          // map name 
      file >> nameToIdAndHdl.second.first;   // map ID

      file >> mapHdlStr;                     // map handled
      if ( mapHdlStr.compare("TRUE") == 0 ){
         nameToIdAndHdl.second.second = true;
      }
      else{
         nameToIdAndHdl.second.second = false;  
      }
    
      if ( ! file.eof() ){
         mc2dbg << "OldMapIdByName::readFile. " 
                << nameToIdAndHdl.first << ":" 
                << nameToIdAndHdl.second.first << ":" 
                << nameToIdAndHdl.second.second << endl;
         m_mapIdAndHdlByName.insert(nameToIdAndHdl);
      }
   }
   file.close();

   // Check the read result.
   if ( m_mapIdAndHdlByName.size() == 0 ){
      mc2log << error << "OldMapIdByName::readFile. Found 0 underviews in " 
             << m_fileName << endl;
      return false;
   }
   return true;

} // readFile

bool
OldMapIdByName::cmpUndIDsWithMapFiles( MC2String mapPath,
                                    uint32& highestID,
                                    uint32& nextMapID )
{
   // Check that all underview maps are present in the file.
   highestID = 0;
   OldMapIdByName::mapIdAndHdlByName_t::const_iterator it = 
      m_mapIdAndHdlByName.begin();
   while ( it != m_mapIdAndHdlByName.end() ){
      // Find highest underview ID
      uint32 mapID = it->second.first;
      if ( MapBits::isUnderviewMap(mapID) && ( mapID > highestID ) ){
         highestID = it->second.first;
      }
      ++it;
   }
   nextMapID = MapGenUtil::getNextUndMapID(mapPath);
   if ( (highestID+1) != nextMapID ){
      return false;
   }
   else {
      return true;
   }
      
} // cmpUndIDsWithMapFiles

bool
OldMapIdByName::overviewsContained()
{
   bool result = false;
   OldMapIdByName::mapIdAndHdlByName_t::const_iterator it = 
      m_mapIdAndHdlByName.begin();
   while ( !result && ( it != m_mapIdAndHdlByName.end() ) ){
      uint32 mapID = it->second.first;      
      if ( MapBits::isOverviewMap(mapID) ){
         result = true;
      }
      ++it;
   }
   
   return result;
} // overviewsContained
