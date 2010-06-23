/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDMAP_H
#define OLDMAP_H

#include "config.h"
#include "OldGenericMap.h"

/**
  *   Description of one map in the OldMapCentral-system.
  *
  */
class OldMap : public OldGenericMap {
   public:

      /**
        *   Creates an empty map.
        */  
      OldMap();

      /**
        *   Create a map with given ID.
        *   @param   id    The ID of the new map.
        */  
      OldMap(uint32 id);

      /**
        *   Create a new OldMap. The map is {\bf not} filled with data from 
        *   the file. This must be done by calling the load()-method after
        *   creation.
        *
        *   @param   mapID Id of the map to be loaded.
        *   @param   path The file-path to the map.
        */
      OldMap(uint32 mapID, const char *path);

      /**
        *   Deletes the map.
        */
      virtual ~OldMap();

};

// ========================================================================
//                                      Implementation of inlined methods =

#endif
