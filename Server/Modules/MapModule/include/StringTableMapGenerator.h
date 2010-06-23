/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRINGTABLEMAPGENERATOR_H
#define STRINGTABLEMAPGENERATOR_H

#include "config.h"
#include "MapGenerator.h"

/**
  *   Objects of this class extracts a stingtable ``map'' and sends it 
  *   via socket to the module/server.  After the map is send (or the 
  *   time out limit is reached) the process (thread) ends and the object 
  *   is deleted by the runtime JTC system.
  *
  */
class StringTableMapGenerator : public MapGenerator {
   public:
      /**
        *   Creates one StringTableMapGenerator.
        *
        *   @param   mh The MapHandler to use to retreiving the Map:s
        *   @param   mapID Pointer to the mapID that the stringtable 
        *            should be generated for.
        *   @param   port The port to listen to connection on.
        */
      StringTableMapGenerator(MapHandler *mh,
                         uint32 *mapIDs,
                         uint16 port);

      /** 
        *   Delete this stringtable map generator.
        */
      virtual ~StringTableMapGenerator();

      /**
        *   Performes the actual generation of the "map".
        *
        *   XXX: Include the file with the external file-information.
        */
      virtual void run();
};

#endif

