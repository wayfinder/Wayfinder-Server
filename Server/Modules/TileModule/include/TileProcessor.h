/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_PROCESSOR_H
#define TILE_PROCESSOR_H

#include "Processor.h"
#include <map>
#include <memory>

class TileMapCreator;
class ReplyPacket;
class TileMapRequestPacket;
class TileMapReplyPacket;
class ServerTileMapFormatDesc;
class STMFDParams;
class TileMapPOIRequestPacket;

/**
 *    Processes RequestPackets. 
 */
class TileProcessor : public Processor {
   public:
      /**
       * Creates a new TileProcessor with a vector of loaded maps
       *
       * @param loadedMaps The standard list of loaded maps for modules.
       */
      TileProcessor( MapSafeVector* loadedMaps );

      virtual ~TileProcessor();
 
      
      /**
       * Returns the status of the module.
       *
       * @return 0.
       */
      int getCurrentStatus();

protected:
       /**
       * Handles a request and returns a reply, can be NULL.
       *
       * @param p The packet to handle.
       * @return A reply to p or NULL if p has unknown packet type.
       */
      Packet *handleRequestPacket( const RequestPacket& p,
                                   char* packetInfo );

private:
   ReplyPacket* handleTileMapPOIRequest( const TileMapPOIRequestPacket& packet,
                                         char* packetInfo ) const;

      /**
       *    Get the ServerTileMapFormatDesc identified by the
       *    settings.
       */
      const ServerTileMapFormatDesc* getTileMapFormatDesc( 
                     const STMFDParams& settings );
      /**
       *    Handle the tilemap request.
       */
      TileMapReplyPacket*  handleTileMapRequest( 
               const TileMapRequestPacket* tileReq,
               char* packetInfo );
      

      /// The tilemap creator.
      auto_ptr<TileMapCreator> m_tileMapCreator;

      /**
       *   The currently loaded stmfd:s, identified by STMFDParams.
       */
      map<STMFDParams, ServerTileMapFormatDesc*> m_stmfds;
};

#endif

