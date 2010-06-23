/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAPPROPERTIES_H
#define TILEMAPPROPERTIES_H

#include "config.h"

/**
 * Filtering and scale level settigns for vector tile maps. 
 *   
 */
class TileMapProperties {
 public:
   /** Used for controlling the filtering in MapModule of area features on all
    *  scale levels.
    *
    *  This factor is multiplied with a meter value, defining maximum 
    *  deviation, so the larger this value is, the more the areas get filtered.
    */
   static const float areaFilterFactor;

   /** Used for controlling the filtering in TileModule of streets on all scale
    *  levels after merging them.
    * 
    *  This factor defines maximum deviation as number of pixels at the most
    *  zoomed out view of each scale level, when showing the map on a 200 px
    *  high screen. You get the number of pixels at the most zoomed in view of
    *  each scale level by multiplying this value by 3.
    *
    *  The larger this value is the more the streets are filtered.
    */
   static const float streetFilterFactor;


   /**
    * Used for setting until what GFX scale level the MapModule includes area
    * features such as built up areas and water.
    */
   static const int32 areaGfxThreshold;


}; //class TileMapProperties


#endif // TILEMAPPROPERTIES_H
