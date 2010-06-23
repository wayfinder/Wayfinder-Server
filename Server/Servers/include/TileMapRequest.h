/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAPREQUEST_H
#define TILEMAPREQUEST_H

#include "config.h"
#include "Request.h"
#include "ServerTileMapFormatDesc.h"
#include "MC2String.h"
#include "FilteredGfxMapRequest.h"
#include "TileMapBufferHolder.h"

class RequestPtrOrRequestID;
class TileMapParams;
class TileMap;
class RouteReplyPacket;
class TopRegionRequest;

/**
 *   A Request that creates a TileMap.
 */
class TileMapRequest : public FilteredGfxMapRequest {
public:
   typedef vector<TileMapBufferHolder> TileBuffers;
   /**
    *   Creates a new TileMapRequest which is inside another request.
    *   @param requestOrID Id of the request or the parent request.
    *                      Automatically converts from uint16 or Request*.
    *   @param mapDesc     A reference to the tilemapformatdesc.
    *   @param params      The parameters.
    *   @param routeReplyPack The route replypacket. May be NULL if
    *                         the route layer is not included.
    *   @param topReq Pointer to valid TopRegionRequest with data
    */
   TileMapRequest( const RequestData& requestOrID,
                   const ServerTileMapFormatDesc& mapDesc,
                   const TileMapParams& params,
                   const RouteReplyPacket* routeReplyPack,
                   const TopRegionRequest* topReq,
                   const MapRights& rights = MapRights() );

   /**
    *   Destroys the request.
    */
   virtual ~TileMapRequest();
   
   ///   Temporary method that gets the empty importances as a bitfield. 
   virtual uint32 getEmptyImps() const;

   /**
    *    Get the requested tilemap buffer.
    */
   virtual const TileMapBufferHolder& getTileMapBuffer() const;
   
   /**
    *   Get the extra tilemap buffers, except the one actually requested
    *   (that one can found in getTileMapBuffer()).
    *   
    *   FIXME: Let this method return all the tilemap buffers instead.
    *   
    */
   virtual void getTileMapBuffers( TileBuffers& holder ) const;
   
private:
   
   /**
    *   Handles that the FilteredGfxMapRequest request is done.
    *   Overrides the super class implementation.
    */
   void handleFilteredGfxMapRequestDone();

};

#endif
