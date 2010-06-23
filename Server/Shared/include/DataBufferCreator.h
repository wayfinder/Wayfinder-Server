/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DATABUFFERCREATOR_H
#define DATABUFFERCREATOR_H

class DataBuffer;

#include "MC2String.h"

/**
 * Creates DataBuffer from file or URL
 */
class DataBufferCreator {
public:
   /**
    *    Runs the program specified by command and tries to
    *    fill the DataBuffer from it.
    *    @return A databuffer if the loading was successful,
    *            NULL if failure.
    */
   static DataBuffer* getDataBufferFromProgram( const char* command );
   /**
    * Loads map into a DataBuffer
    * @param filename the filename
    * @return data buffer pointer
    */
   static DataBuffer* loadFromFile( const char* filename );
   /**
    * Loads data from url.
    * @param filename the filename
    * @return data buffer pointer
    */
   static DataBuffer* loadFromURL( const char* filename );

   /**
    *   Loads from map server. Uses MAP_PATH_URL.
    *   If mapSet == MAX_UINT32 then the properties will be used.
    */
   static DataBuffer* loadFromMapServer( const MC2String& filename );

   /**
    *   Loads a map or index.db using the property values and the
    *   filename. Will loadFromMapServer if no file exists.
    */
   static DataBuffer* loadMapOrIndexDB( const MC2String& filename,
                                        uint32 mapSet = MAX_UINT32 )
      throw( MC2String );
      
   /**
    *   @param mapSet Give this to specify map set, otherwise map set is
    *                 read from mc2.prop.
    *   @param filename The file name of the file to get path of.
    *   @return Returns a full path to a map file or index.db file.
    */
   static MC2String getMapOrIdxPath( const MC2String& filename, 
                                     uint32 mapSet = MAX_UINT32);


};


#endif 
