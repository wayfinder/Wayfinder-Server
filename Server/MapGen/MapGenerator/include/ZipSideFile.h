/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ZIPSIDEFILE_H
#define ZIPSIDEFILE_H

#include "config.h"
#include <map>
#include <vector>
#include <set>


#include "GDFID.h"
#include "SectionPair.h"
#include "MC2String.h"
class GDFRef;
class OldItem;
class MC2Coordinate;

/**
  *   ZipSideFile description.
  *
  */
class ZipSideFile
{
 public:
   
   /**
    * Constructor.
    */
   ZipSideFile();
   


   /**
    * @return Returns zip names of all zip codes located in one of the
    *         sections of the gdfRef of this ZipSideFile object.
    */
   vector<MC2String> getMapZipItemNames() const;

   /**
    * @return Returns true if the zip code is inside one of the sections
    *         of the gdfRef of this ZipSideFile object.
    */
   bool zipInSectionOfMap(MC2String zipName);


   /**
    * @param zipCodeName The name of the zip code to find the center 
    *        coordinate of.
    * @return The center coordinate of the zip code. An invalid coordinate
    *         is returned if the zip code is not present.
    */
   MC2Coordinate getZipCenterCoord(MC2String zipCodeName) const;


   /**
    * Load the zip code file with zip code names and coordinages.
    *
    * @param zipPOIFilePath Path to a file including the zip data.
    * @param sectionID Give section ID if this is a sectioned zip file.
    *        Not needed otherwize.
    */
   bool loadZipPOIFile( const MC2String zipPOIFilePath, 
                        uint32 sectionID = 0 );


 private:
   /// Factor for converting between shifted wgs84 degrees and MC2
   static const float64 COORD_FACTOR;

   /**
    * Key:  Full GDF ID (dataset, section, feature)
    * Data: Zip code string.
    */
   typedef multimap<SectionPair, MC2String> zipBySection_t;

   /**
    * This one may include multiple zip codes per SSI feature.
    */
   zipBySection_t m_zipCodesBySection;
  
   /**
    * Key:  Zip code name
    * Data: Zip code center point coordinate.
    */
   typedef map<MC2String, MC2Coordinate> coordByZipName_t;

   /**
    * Used for finding center point of a zip code.
    */
   coordByZipName_t m_coordByZipName;

   /**
    * The section IDs of the real sections, not neighbouring sections,
    * of each zip is stored in this one.
    */
   multimap<MC2String, uint32> m_trueSectionsByZip;

   /**
    * This one is filled when the gdf ref is changed, so it always
    * includes the right section IDs.
    */
   set<uint32> m_gdfRefSectionIDs;


   /// Used for finding GDF ID of items.
   const GDFRef* m_gdfRef;

   /**
    * Since zip key of section file does not include the space,
    * this one is needed to map the section to the right zip name.
    */
   map<MC2String, MC2String> m_zipNameByKey;

   
   /**
    * Converts an integer to a string.
    * @return A string created from the value in i.
    */
   MC2String int2str (int32 i) const;
   


};
#endif // ZIPSIDEFILE_H
