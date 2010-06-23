/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_MAP_CREATOR_H
#define TILE_MAP_CREATOR_H

#include "config.h"
#include "GfxFeatureMap.h"
#include "GfxFeature.h"
#include "ServerTileMapFormatDesc.h"
#include "TileMap.h"
#include "ServerTileMap.h"
#include "TileFeature.h"
#include "TileMapParams.h"

#include "MC2BoundingBox.h"
#include "POIOverlapDetector.h"
#include "POICache.h"

#include <memory>

class TileImportanceNotice;
class ServerTileMapFormatDesc;
class GfxFeature;
class TileMapBufferHolder;
class POIImageIdentificationTable;

/**
 *    Class used to create TileMaps.
 *
 */
class TileMapCreator {
public:
   explicit TileMapCreator( const ServerTileMapFormatDesc& desc );
   ~TileMapCreator();

   /// Cache of pois, prevents loading from disk when not needed.
   typedef std::map<MC2String, POIData> poiCacheMap_t;
   
   /**
    *    Create a tilemap from the gfx map.
    *    @param   params      The tilemap params.
    *    @param   desc        The map description.
    *    @param   importance  The tile map importance notice.
    *    @param   gfxMap      The gfxmap to make a tile map from. This
    *                         gfxMap can be changed if it contains e.g.
    *                         self-intersecting polygons.
    *    @return  A new tile map.
    */
   TileMap* createTileMap( const TileMapParams& params,
                           const ServerTileMapFormatDesc* desc,
                           const TileImportanceNotice& importance,
                           GfxFeatureMap* gfxMap );

   static TileMapBufferHolder*
   createOceanTileMap( const MC2SimpleString& str,
                       const ServerTileMapFormatDesc& desc );
private:
   /**
    * Setup special_custom_poi types image and real feature type.
    * @param poi the current gfx feature.
    * @param tilemap the current tilemap
    * @param desc descriptor for tilemaps
    * @param tileFeatureType
    * @return new feature type
    */
   inline WritableTileFeature* 
   setupCustomPOI( const GfxPOIFeature& poi,
                   WritableTileMap& tileMap,
                   const ServerTileMapFormatDesc& desc,
                   int32 tileFeatureType ) const;
   /**
    *    Create a tilefeature from the gfx feature / polygon.
    *    @param   desc        The map description.
    *    @param   tileMap     The tile map for the tile feature.
    *    @param   gfxFeature  The gfx feature to create a tile feature from.
    *    @param   gfxPolygon  The polygon of the gfx feature to create
    *                         a tile feature from.
    *    @return  A new TileFeature or NULL if something went wrong.
    */
   TileFeature* createTileFeature( const ServerTileMapFormatDesc* desc,
                                   WritableTileMap* tileMap,
                                   const GfxFeature* gfxFeature,
                                   const GfxPolygon* gfxPolygon );
   /**
    *    Create a ocean feature (background) covering all of the gfxmap.
    *    @param   gfxMap   The gfxmap.
    *    @param   tileMap  The tile map for the ocean feature.
    *    @param   desc     The map format description.
    *    @return  The ocean feature.
    */
   static TileFeature* createOceanFeature( WritableTileMap& tileMap,
                                           const TileMapFormatDesc* desc,
                                           const MC2BoundingBox& bbox );

   POICache m_poiDataCache;
   /// Translation table for custom poi images
   auto_ptr<POIImageIdentificationTable> m_poiImageTable;
};


// =======================================================================
//                                     Implementation of inlined methods =


#endif // TILE_MAP_CREATOR_H

