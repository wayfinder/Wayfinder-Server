/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPTOPREGION_H
#define MAPTOPREGION_H

#include "TopRegionMatch.h"

/**
 *    Represents a top region with some additional 
 *    information / functionality.
 *
 */
class MapTopRegion : public TopRegionMatch 
{
public:

   /**
    *    Create a new MapTopRegion. Add names using addName method.
    *    @param id   The id of this MapTopRegion.
    *    @param type The type of this MapTopRegion.
    */
   explicit MapTopRegion( uint32 id = MAX_UINT32,
                          topRegion_t type = TopRegionMatch::country );
   
   /**
    * Creates a new MapTopRegion.
    * 
    * @param id The id of this top region.
    * @param type The type of top region.
    * @param idTree Tree of map and item ids.
    * @param bbox Boundingbox of the region.
    * @param names The names of the region.
    */
   MapTopRegion( uint32 id, 
                 topRegion_t type,
                 const ItemIDTree& idTree,
                 const MC2BoundingBox& bbox,
                 const NameCollection& names );
   
   /**
    *    Loads the MapTopRegion from the supplied DataBuffer.
    *    @param mapSet Map Set ID to use, if default value MAX_UINT32 it is
    *    ignored
    */
   bool load(DataBuffer* buf, uint32 mapSet = MAX_UINT32);
   
   /**
    *    Saves the MapTopRegion to the supplied DataBuffer.
    */
   bool save(DataBuffer* buf) const;
   
};

#endif

