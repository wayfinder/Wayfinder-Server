/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXCOUNTRYMAPGENERATOR_H
#define GFXCOUNTRYMAPGENERATOR_H

#include "GenericGfxMapGenerator.h"

class Item;
class GenericMap;


/**
  *   Objects of this class extracts an geographic "map" for one country
  *   and sends it via socket to the module/server.
  *
  */
class GfxCountryMapGenerator : public GenericGfxMapGenerator {
   public:
      /**
        *   Creates one GfxCountryMapGenerator.
        *
        *   @param   mh    The MapHandler to use to retreiving the Map:s
        *   @param   mapID Pointer to the mapID that the gfx country map
        *                  should be generated for.
        *   @param   port  The port to listen to connection on.
        */
      GfxCountryMapGenerator(MapHandler *mh,
                             uint32 *mapIDs,
                             uint32 port);
      
      /**
       *    Delete this map generator.
       */
      virtual ~GfxCountryMapGenerator();

      /**
        *   Performes the actual generation of the map.
        *
        *   XXX: Include the file with the external file-information.
        */
      virtual void run();
      
   protected:
      /**
       *    Abstract method.
       *    Determine if to include a item on given zoomlevel.
       *
       *    The caller make sure that in case a StreetItem is 
       *    sent to this method, all polygons of that StreetItem must be
       *    checked seperately (with the polygon parameter set).
       *    This is the case since different polygons of a StreetItem
       *    can have different roadclasses.
       *    
       *    @param item    The item to check.
       *    @param theMap  The map containing the item.
       *    @param reduceGfxToPoint Outparameter, set to true if the 
       *                            graphics should be reduced to a
       *                            single point.
       *    @param polygon Only need to be supplied when passing a 
       *                   StreetItem as item. Which polygon of the
       *                   StreetItem to check if it is to be included.
       *    @return True if item should be included, false otherwise.
       */
      virtual bool includeItem(const Item* item, GenericMap* theMap, 
                               bool& reduceGfxToPoint, 
                               uint16 polygon = MAX_UINT16);

};

#endif
