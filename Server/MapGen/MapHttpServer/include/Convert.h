/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CONVERT_H
#define CONVERT_H

#include "MC2String.h"

/**
 * Converts between Old and New maps
 */
class MapModuleNotice;

struct Convert {
   /**
    * Convert between src file to dest file
    * @param srcFilename the source map or index.db
    * @param destFilename the sourcemap or index.db
    * @param mapNotice if this is specified the creation time 
    *  will be asserted to the loaded src map creation time
    */   
   static void convertFile( const MC2String& srcFileName, 
                            const MC2String& destFileName,
                            const MapModuleNotice* mapNotice = 0);
   /**
    * Sets destPath to MAP_SERV_DEST_PATH and
    * using mapSet to determine which MAP_SERV_SOURCE_PATH_<mapSet>
    * that srcPath would be assign to.
    * @param mapSet the map set, so the corrent MAP_*_<num> gets selected
    * @param destPath holds destination path. MAP_SERV_DEST_PATH
    * @param srcPath holds source path. MAP_SOURCE_PATH_<mapSet>
    */
   static void getMapPaths( uint32 mapSet,
                            MC2String& destPath, 
                            MC2String& srcPath );
   /**
    * Loads index.db and converts all maps inside it 
    * to destination path specified by MAP_SERV_DEST_PATH
    */ 
   static void convertFromIndex( uint32 mapSet, const vector<uint32>& mapIDs );
   /**
    * Converts a set of mapSets with an optional list of mapIDs.
    */
   static void convertSet( const vector<uint32>& mapIDSet,
                           const vector<uint32>& mapIDs );
   /**
    *   Loads all index.db and converts the maps inside it.
    */
   static void convertAll();

   /**
    *   Creates an OldGenericMap filename
    */
   static MC2String getSourceFileName( const MC2String& srcPath,
                                       uint32 mapID );

   /**
    *   Creates a GenericMap filename.
    */
   static MC2String getDestFileName( const MC2String& dstPath,
                                     uint32 mapID );
   

};

#endif 
