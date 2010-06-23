/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FILTEREDGFXMAPREQUEST_H
#define FILTEREDGFXMAPREQUEST_H

#include "config.h"
#include "Request.h"
#include "ServerTileMapFormatDesc.h"
#include "TileMapPacket.h"
#include "MapRights.h"

class GfxFeatureMapImageRequest;
class PacketContainer;
class RequestPtrOrRequestID;
class MapSettings;
class TileMapParams;
class RouteReplyPacket;
class GfxFeatureMap;
class TopRegionRequest;
class RedLineSettings;

/**
 *   A Request that creates a filtered gfxmap.
 */
class FilteredGfxMapRequest : public RequestWithStatus {
public:

   /**
    *   Creates a new FilteredGfxMapRequest which is inside another request.
    *
    *   @param requestOrID Id of the request or the parent request.
    *                      Automatically converts from uint16 or Request*.
    *   @param redLineSettings Settings for the red line in java client.
    *   @param mapDesc     A reference to the tilemapformatdesc.
    *   @param bbox        The boundingbox of the resulting gfxmap.
    *   @param tileMapLayerID Tilemap layer id, (map, route, poi:s etc).
    *   @param tileMapImportanceNbr Tilemap importance nbr. This determines
    *                                which features that will be kept in
    *                                the resulting map.
    *   @param pixelToMeter Pixel to meter factor.
    *   @param routeReplyPack The route replypacket. May be NULL if
    *                         the route layer is not included.
    *   @param   topReq       Pointer to valid TopRegionRequest with data
    *   @param removeNames  If to skip the names of the gfxmap.
    *   @param resultMap    If the result should be a gfxmap or tilemap.
    *   @param rights the map rights for pois
    */
   FilteredGfxMapRequest( const RequestData& requestOrID,
                          const RedLineSettings& redline,
                          const ServerTileMapFormatDesc& mapDesc,
                          const MC2BoundingBox& bbox,
                          int tileMapLayerID,
                          int tileMapImportanceNbr,
                          int pixelToMeter,
                          const RouteReplyPacket* routeReplyPack,
                          const TopRegionRequest* topReq,
                          bool removeNames = false,
                          TileMapRequestPacket::resultMap_t resultMap = 
                          TileMapRequestPacket::gfxmap_t,
                          const MapRights& rights = MapRights() );

   /**
    *   Creates a new FilteredGfxMapRequest which is inside another request.
    *
    *   @param requestOrID Id of the request or the parent request.
    *                      Automatically converts from uint16 or Request*.
    *   @param mapDesc     A reference to the tilemapformatdesc.
    *   @param params      The tilemap parameters.
    *   @param routeReplyPack The route replypacket. May be NULL if
    *                         the route layer is not included.
    *   @param   topReq       Pointer to valid TopRegionRequest with data
    *   @param resultMap    If the result should be a gfxmap or tilemap.
    */
   FilteredGfxMapRequest( const RequestData& requestOrID,
                          const ServerTileMapFormatDesc& mapDesc,
                          const TileMapParams& params,
                          const RouteReplyPacket* routeReplyPack,
                          const TopRegionRequest* topReq,
                          TileMapRequestPacket::resultMap_t resultMap = 
                          TileMapRequestPacket::tilemap_t,
                          const MapRights& rights = MapRights() );

   /**
    *   Destroys the request.
    */
   virtual ~FilteredGfxMapRequest();
   
   /**
    *   Returns the status of the request.
    */
   StringTable::stringCode getStatus() const;

   /**
    *   Processes a packet.
    */
   void processPacket( PacketContainer* pack );

   /**
    *   Returns true when the request is done.
    */
   bool requestDone();

   /**
    *   Returns a pointer to the resulting GfxFeatureMap. 
    */
   const GfxFeatureMap* getGfxFeatureMap() const;
   /// @return map rights 
   const MapRights& getMapRights() const;
   const TileMapParams& getTileMapParams() const { return m_param; }
protected:
   /**
    *   Initializes the request (to be used by constructor).
    */
   void init( const RedLineSettings* redline, 
              const TileMapParams* param,
              const MapRights& rights );
   
   /**
    *   The current status of the request.
    */
   StringTable::stringCode m_status;

   /**
    *   Handles that the gfxmap request is done.
    */
   void handleGfxRequestDone();
   
   /**
    *   Handles that the request is done.
    */
   virtual void handleFilteredGfxMapRequestDone();
 
   /**
    *   Describes what we should be doing right now.
    */
   enum state_t {
      /// Means that we are currently using the GfxFeatureMapRequest.
      USING_GFX_REQUEST       = 100,
      /// Means that we are using this request.
      GENERIC_STATE           = 101,
      /// The request is done, but with an error.
      ERROR                   = 404,
      /// The request is done and ok.
      DONE_OK                 = 1000
   } m_state;
   
   /**
    *   Helper function that can be used to print/not print state
    *   changes.
    */
   void setState( state_t newState, int line );
   
   /**
    *   The GfxFeatureMapRequest.
    */
   GfxFeatureMapImageRequest* m_gfxFeatureRequest;

   /**
    *   The MapSettings for the GfxFeatureMapImageRequest.
    */
   MapSettings* m_mapSettings;
   
   /**
    *   The GfxFeatureMap.
    */
   GfxFeatureMap* m_gfxMap;
   
   /**
    *    The tilemap param.
    */
   TileMapParams m_param;

   /**
    *    The routereply packet.
    */
   const RouteReplyPacket* m_routeReplyPack;
   
   /**
    *    Tilemapformat description.
    */
   const ServerTileMapFormatDesc& m_mapDesc;

   /**
    *    If to remove names.
    */
   bool m_removeNames;

   /**
    *    The boundingbox to extract maps from.
    */
   MC2BoundingBox m_bbox;

   /// pointer to topregionrequest
   const TopRegionRequest* m_topReq;

   /// TileMap buffer holder.
   vector<TileMapBufferHolder> m_tileMapBufferHolder;

   /// The packet containers to delete in the destructor.
   vector<PacketContainer*> m_packetContainersToDelete;

   /// If a tilemap or gfxmap should be requested from the module.
   TileMapRequestPacket::resultMap_t m_resultMapType;
   
   /**
    *   The pixel to meter factor used when filtering.
    *   If this factor is -1, then the actual factor will be
    *   calculated from the tilemap params.
    */
   int m_pixelToMeter;
};

#endif
