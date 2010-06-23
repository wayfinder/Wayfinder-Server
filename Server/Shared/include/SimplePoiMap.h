/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SIMPLE_POI_MAP_H
#define SIMPLE_POI_MAP_H

#include "config.h"
#include "MC2String.h"
#include <memory>

class DrawingProjection;
class GfxFeatureMap;
class ServerTileMapFormatDesc;
class SharedBuffer;
class POIImageIdentificationTable;

namespace {
   class SimplePoiNotice;
}

/**
 *   The simple square poi map for java and web clients.
 */
class SimplePoiMap {
public:

   /**
    *   Creates SimplePoiMap.
    *   Does not copy the GfxFeatureMap or ServerTileMapFormatDesc
    *   so keep them around while using the SimplePoiMap.
    * @param featMap the raw data.
    * @param desc server tile map format descriptor for feature types and
    *             images.
    *
    * @param proj drawing projection for the poi image map
    * @param imageTable
    */
   SimplePoiMap( const GfxFeatureMap& featMap,
                 const ServerTileMapFormatDesc& desc,
                 const DrawingProjection& proj,
                 const POIImageIdentificationTable& imageTable );

   /// Deletes the bytes
   ~SimplePoiMap();

   /// Returns a buffer containing the saved map.
   const SharedBuffer* getAsBytes() const;

   /// Returns a buffer containing the saved maps. Buffer must be deleted.
   SharedBuffer* getAsNewBytes();
   
private:
   /**
    * Determine the new feature type for the special custom poi type
    * with custom image.
    * @param featureType special custom poi type
    * @return new feature type id for simple poi desc table.
    */
   uint16 getSpecialFeatureType( uint32 poiType,
                                 const MC2String& customImage ) const;
   /// Contains the data.
   mutable auto_ptr<SharedBuffer> m_bytes;
   /// Original map
   const GfxFeatureMap& m_gfxMap;
   /// TileMapFormatDesc
   const ServerTileMapFormatDesc& m_desc;
   /// Drawing projection
   const DrawingProjection& m_proj;
   const POIImageIdentificationTable& m_imageTable;
};

#endif
