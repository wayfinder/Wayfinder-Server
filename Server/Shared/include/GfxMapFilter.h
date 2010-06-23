/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFX_MAP_FILTER_H
#define GFX_MAP_FILTER_H

#include "config.h"

class ServerTileMapFormatDesc;
class TileMapParams;
class GfxFeatureMap;
class Stack;
class GfxData;

namespace GfxMapFilter {

/**
 *    Does the needful merging / filtering / removal of uninteresting
 *    items of the gfxmap.
 *
 *    @param gfxMap        The gfxmap to filter. Note that this
 *                         gfxmap will be modified by the method,
 *                         and should not be used to anything else
 *                         afterwards.
 *    @param mapDesc       The map format description.
 *    @param   param       The TileMapParam for the importance.
 *    @param meterToPixelFactor  Meter to pixel factor. Used by
 *                               the filtering.
 *    @param removeNames   If to remove all names in the gfxmap.
 *    @return  A new:ed filtered GfxMap. Note that this gfxmap
 *             must be deleted by the caller of this method.
 */
GfxFeatureMap* filterGfxMap( GfxFeatureMap& gfxMap, 
                             const ServerTileMapFormatDesc& mapDesc,
                             const TileMapParams& param,
                             float64 meterToPixelFactor,
                             bool removeNames = false );

/**
 * Determines the next index to use in the polygon, i.e whether to remove
 * any indeces in between to do a straighter line.
 *
 * @param filteredIndeces Contains index in to gfx polygon.
 * @param gfx Contains the coordinates
 * @param poly Polygon index in Gfx to filter.
 * @param startIndexIn Start index in filteredIndeces.
 * @param distanceCutOff The maximum distance before
 * @return next index to be used in filteredIndeces.
 */
uint32 getFirstAngleDifferenceIndex( const Stack& filteredIndeces,
                                     const GfxData& gfx,
                                     uint32 poly,
                                     uint32 startIndexIn,
                                     float64 distanceCutOff );
} 

#endif // GFX_MAP_FILTER_H
