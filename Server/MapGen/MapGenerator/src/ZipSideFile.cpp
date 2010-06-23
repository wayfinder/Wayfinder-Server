/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ZipSideFile.h"
#include "MC2String.h"
#include "MapGenUtil.h"
#include "GDFID.h"
#include "STLStringUtility.h"
#include "Utility.h"

#include "StringUtility.h"
#include "MC2Coordinate.h"
#include <math.h>




/* How the coordinate factor used below is calculated
   for 5 decimal places.
      
   (1 /100000.0) * (PI / 180.0) * ((float64) pow(2,32)/(2*PI)) =
   = (1/100000)*(1/360)*2^32
   = 2^32/(360*100000)
   = 2^23/ 70312.5; */

// This one is used for coord data that has 7 decimal places
const float64 ZipSideFile::COORD_FACTOR = ( (float64) pow(2.0, 23.0) )
                                               / 7031250;



ZipSideFile::ZipSideFile()
{
   m_gdfRef=NULL;

} // ZipSideFile





bool
ZipSideFile::loadZipPOIFile( const MC2String zipPOIFilePath, 
                             uint32 sectionID /*= 0*/ )
{
   mc2dbg << "ZipSideFile::loadZipPOIFile: " << zipPOIFilePath
          << endl; 
   
   bool result = true;

   // Read all rows and columns.
   vector< vector<MC2String> > fileTable;
   uint32 nbrCols = 5;
   uint32 latCol = 4;
   uint32 lonCol = 3;

   MC2String colSep = "|";
   result = result && 
      MapGenUtil::loadTxtFileTable( zipPOIFilePath,  nbrCols, colSep,
                                    fileTable );
   if ( !result ){
      mc2log << error << "Failed to read " << zipPOIFilePath << endl;
   }
   mc2dbg << "Loaded zip code POI file, " << fileTable.size() 
          << " rows." << endl; 
   
   
   // Set the data of the zip code file.
   for ( uint32 i=0; i<fileTable.size(); i++){

      // Extract zip code name
      MC2String space;
      uint32 fieldSize = fileTable[i][0].size();
      mc2dbg8 << "Last char: " << fileTable[i][0][fieldSize-1] << endl;

      MC2String zipCodeName;
      MC2String zipCodeKey;
      if ( fieldSize == 0 ){
         // This is a sectioned POI file. No zipCodeKey available.
         zipCodeName = fileTable[i][2];
      }
      else {
         if ( fileTable[i][0][fieldSize-1] != ' ' ){
            // Only add the space if not already present.
            space+=" "; 
         }
         zipCodeName = fileTable[i][0] + space + fileTable[i][1];
         zipCodeKey = fileTable[i][2];
      }


      // Extract center coordinate
      int32 lat = 
         static_cast<int32>
         (floor(0.5 +
                strtol( fileTable[i][latCol].c_str(), 
                        (char**) NULL, 10 ) * 
                COORD_FACTOR) );
      int32 lon = 
         static_cast<int32>
         (floor(0.5 + 
                strtol( fileTable[i][lonCol].c_str(), 
                        (char**) NULL, 10 ) * 
                COORD_FACTOR ) );

      mc2dbg8 << "Coord: " << fileTable[i][4] << ":" << fileTable[i][3] 
             << " - " << lat << ":" << lon << endl;

      // Make first in word capital is run on zip code item names. Here
      // as well, to make them match.
      MC2String itemZipCodeName = 
         MapGenUtil::handleItemNameCase(zipCodeName);

      // Fill m_coordByZipName:
      m_coordByZipName.insert(make_pair(itemZipCodeName,
                                        MC2Coordinate(lat, lon)));
      mc2dbg8 << "Adding: " << zipCodeName << " - " 
              << MC2Coordinate(lat, lon) << endl;

      // Fill m_zipNameByKey
      if ( fieldSize > 0 ){
         // This is read from the original zip code file.
         m_zipNameByKey.insert(make_pair(zipCodeKey, itemZipCodeName));
      }
      else {
         // This is read from the sectioned zip code file.
         MC2String isNSorS = fileTable[i][1];
         bool neighbour = true;
         if ( isNSorS == MC2String("S") ){
            neighbour = false;
         }
         if ( !neighbour ){
            m_trueSectionsByZip.insert(make_pair(itemZipCodeName, 
                                                 sectionID));
         }
      }
      //mc2dbg8 << zipCodeName << ":" << zipCodeKey << endl;

   }
   mc2dbg << "Size of m_coordByZipName: " << m_coordByZipName.size()
          << endl;

   return result;
} // loadZipPOIFile



vector<MC2String> 
ZipSideFile::getMapZipItemNames() const
{
   set<MC2String> allMapZips;

   if ( m_zipCodesBySection.size() == 0 ){
      mc2dbg << "ZipSideFile::getMapZipItemNames: From m_coordByZipName" 
             << " size: " << m_coordByZipName.size() << endl;
      
      for ( coordByZipName_t::const_iterator zipIt = m_coordByZipName.begin();
            zipIt != m_coordByZipName.end();
            ++zipIt ){
         allMapZips.insert(zipIt->first);
      }
   }  
   else {
      mc2dbg <<  "ZipSideFile::getMapZipItemNames: From m_zipCodesBySection"
             << endl;
                   
      for ( zipBySection_t::const_iterator zipIt = 
            m_zipCodesBySection.begin();
            zipIt != m_zipCodesBySection.end(); ++zipIt ){

         set<uint32>::iterator secIt = 
            m_gdfRefSectionIDs.find(zipIt->first.getSection());
         if (secIt != m_gdfRefSectionIDs.end() ){
            allMapZips.insert(zipIt->second);
         }
      }
   }   
   vector<MC2String> result(allMapZips.size());
   copy(allMapZips.begin(), allMapZips.end(), result.begin());
   return result;
}


bool
ZipSideFile::zipInSectionOfMap(MC2String zipName)
{
   bool result = false;

   multimap<MC2String, uint32>::const_iterator zipIt = 
      m_trueSectionsByZip.lower_bound(zipName);
   while (zipIt != m_trueSectionsByZip.upper_bound(zipName)){
      uint32 sectionID = zipIt->second;
      if ( m_gdfRefSectionIDs.find(sectionID) !=
           m_gdfRefSectionIDs.end() ){
         result = true;
      }
      ++zipIt;
   }


   return result;
} // zipInSectionOfMap





MC2Coordinate
ZipSideFile::getZipCenterCoord(MC2String zipCodeName) const
{
   coordByZipName_t::const_iterator zipIt = 
      m_coordByZipName.find(zipCodeName);
   if (zipIt != m_coordByZipName.end() ){
      return zipIt->second;
   }
   else {
      return MC2Coordinate();
   }
} // getZipCenterCoord


MC2String
ZipSideFile::int2str (int32 i) const
{
   MC2String result;
   return STLStringUtility::int2str (i, result); 
} 
