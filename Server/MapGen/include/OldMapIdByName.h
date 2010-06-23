/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDMAPIDBYNAME_H
#define OLDMAPIDBYNAME_H


#include <map>
#include "config.h"
#include "MC2String.h"

/**
 *   Class used by both MapModule and GenereateMapServer for map generation
 *   specific tasks.
 *
 *   NB. This class is not thread safe.
 *
 */
class OldMapIdByName {
 public:

   /**
    * Stores underview map name as key mapped to underview map ID and a
    * bool telling if the map has been handled as value. An underview map
    * has been handled when its data has been added to an overview.
    */
   typedef map< MC2String, pair< uint32, bool> > mapIdAndHdlByName_t;

   /**
    * Creates an OldMapIdByName object.
    *
    * @param fileName The file name used when reading and writing map ID
    *                 by map name file.
    */
   OldMapIdByName( MC2String fileName );

   /**
    * @return Returns a writeable structure reprecenting the content of 
    *         the file storing map ID by map name. Set a referens variable
    *         to the returned structure and edit the content of it to 
    *         influence what is written in the file.
    */
   mapIdAndHdlByName_t& getWriteableMapIdByAndHdlName();
   
   /** 
    *  @return True if the map ID by map name file exists on disc.
    *          Otherwise false.
    */
   bool fileExists();
      
   /**
    * Inits internal map ID by map name from maps found in mapPath. All
    * handled status is set to false. Writes the result to disc in a file.
    *
    * @param mapPath Place to load maps from.
    *
    * @return True if successful otherwise false.
    */
   bool initAndWriteFile(MC2String mapPath);

   
   /** 
    *  Writes the content of this object in a file.
    *  @return True if successful otherwise false.
    */
   bool writeFile();
   
   /** 
    *  Reads the content of the map ID by map name file and inits the
    *  object with its content.
    *
    *  @return True if successful otherwise false.
    */
   bool readFile();
   
   /**
    *  Compare the underview IDs stored in this object  with the 
    *  underview maps stored in mapPath and make sure no overview maps
    *  are present in mapPath directory.
    *
    *  @param mapPath The directory where the underview maps to compare 
    *                 with are stored.
    *  @param highestID Outparameter The highest map ID found in 
    *                                mapIdAndHdlByName.
    *  @param nextMapID Outparameter The next available map ID in mapPath.
    *
    *  @return True if the underview IDs in mapIdAndHdlByName are exactly
    *          the same as the unverview maps stored in mapPath.
    *          Otherwise false. Also returns false if overview maps
    *          exists in mapPath directory.
    */
   bool cmpUndIDsWithMapFiles( MC2String mapPath,
                               uint32& highestID,
                               uint32& nextMapID );

   /**
    *  Tells whether the objcet contains overview maps.
    *
    *  @return True if overview maps are contained, otherwise false.
    */
   bool overviewsContained();

   
 private:

   /**
    * Stores map ID and handled status, mapped to map name.
    */
   mapIdAndHdlByName_t m_mapIdAndHdlByName;

   /**
    * This is the file name of the file used for storing the content of
    * this object.
    */
   MC2String m_fileName;
   
}; // class OldMapIdByName

#endif // MAPIDBYNAME_H
