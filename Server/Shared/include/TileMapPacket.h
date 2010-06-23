/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAPPACKET_H
#define TILEMAPPACKET_H

#include "config.h"

#include "Packet.h"
#include <vector>
#include "MC2SimpleString.h"

class GFMDataHolder;
class STMFDParams;
class BitBuffer;
class TileMap;
class TileMapBufferHolder;

/// TileMapRequestPacket.
class TileMapRequestPacket : public RequestPacket {
public:

   /// The resulting map type, i.e. filtered gfxmap or tilemap.
   enum resultMap_t {
      gfxmap_t = 0,
      tilemap_t
   };
   
   /**
    *   Creates new TileMapRequestPacket.
    *
    *   @param gfmData     GFMDataHolder containing the gfxfeaturemaps.
    *   @param stmfdParams Params describing the mapformatdesc.
    *   @param removeNames If to remove all names in the map.
    *   @param resultMap_t What kind of resulting map that is wanted,
    *                      tilemap or filtered gfxmap.
    *   @param param       The tilemap param.
    *   @param pixelToMeter   Pixel to meter factor, used when filtering.
    *                         If set to -1, this factor will instead be
    *                         calculated from the tilemap param.
    */
   TileMapRequestPacket( const GFMDataHolder& gfmData,
                         const STMFDParams& stmfdParams,
                         bool removeNames,
                         const resultMap_t& resultMapType,
                         const MC2SimpleString& param,
                         int pixelToMeter );
                  
   /**
    *   Gets the data from the packet.
    *
    *   @param gfmData     GFMDataHolder containing the gfxfeaturemaps.
    *   @param stmfdParams Params describing the mapformatdesc.
    *   @param removeNames If to remove all names in the map.
    *   @param resultMap_t What kind of resulting map that is wanted,
    *                      tilemap or filtered gfxmap.
    *   @param param       The tilemap param.
    *   @param pixelToMeter   Pixel to meter factor, used when filtering.
    *                         If set to -1, this factor will instead be
    *                         calculated from the tilemap param.
    */
   void get( GFMDataHolder& gfmData,
             STMFDParams& stmfdParams,
             bool& removeNames,
             resultMap_t& resultMapType,
             MC2SimpleString& param,
             int& pixelToMeter ) const;
   
};


/// TileMapReplyPacket.
class TileMapReplyPacket : public ReplyPacket {
public:

   /**
    *   Creates a reply packet to a TileMapRequestPacket.
    */
   TileMapReplyPacket( const TileMapRequestPacket* req,
                       const vector<TileMapBufferHolder>& tileMapBufferHolder ); 
   /**
    *   Creates a reply packet to a TileMapRequestPacket in case something
    *   went wrong (status != StringTable::OK).
    */
   TileMapReplyPacket( const TileMapRequestPacket* req,
                       uint32 status );

   /**
    *   Returns the reply data.
    */
   void get( vector<TileMapBufferHolder>& tileMapBufferHolder ) const;
   
};

#endif
