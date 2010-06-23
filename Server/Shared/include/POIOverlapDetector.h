/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POI_OVERLAP_DETECTOR
#define POI_OVERLAP_DETECTOR

#include "config.h"
#include <map>
#include <set>
#include "MC2String.h"
#include "MC2Point.h"
#include "POICache.h" 

class GfxFeatureMap;
class GfxFeature;
class TileMapParams;
class WritableTileMap;
class ServerTileMapFormatDesc;

/**
 *    
 */
class POIOverlapDetector {
   
public:

   /// Cache of pois, prevents loading from disk when not needed.
   typedef std::map<MC2String, POIData> poiCacheMap_t;
   typedef set<const GfxFeature*> avoidSet_t;
   
   /**
    * Checks if pois in the same category is ovelapping each
    * other. If so it removes the latest from the map.
    * @param gfxMap The map containing all the poi data.
    * @param params Needed for getting the lowest scale and for calculating
    *               the tileBox.
    * @param refTileMap Used for snapping the coords of a feature to a correct pixel.
    * @param desc Used for getting info about the feature etc.
    * @param poiCache Cache containing allready calculated poi:s
    * @param avoidSet POIs that shouln't be included in the map is added
    *                 to this list.
    */
   static void removeOverlappedPOIs( GfxFeatureMap& gfxMap, 
                                     const TileMapParams& params,
                                     WritableTileMap& refTileMap,
                                     const ServerTileMapFormatDesc& desc,
                                     POICache& poiCache,
                                     avoidSet_t& avoidSet,
                                     int pixelMargin = 0 );
};

#endif
