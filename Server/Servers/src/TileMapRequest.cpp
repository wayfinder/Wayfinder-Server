/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "TileMapRequest.h"
#include "TileMapParams.h"

#include "TileMap.h"
#include "PacketContainer.h"

// To be able to change this into printing the state
// always use SETSTATE instead of m_state=
#define SETSTATE(s) setState(s, __LINE__)

TileMapRequest::TileMapRequest( const RequestData& requestOrID,
                                const ServerTileMapFormatDesc& mapDesc,
                                const TileMapParams& params,
                                const RouteReplyPacket* routeReplyPack,
                                const TopRegionRequest* topReq,
                                const MapRights& rights )
      : FilteredGfxMapRequest( requestOrID,
                               mapDesc,
                               params,
                               routeReplyPack,
                               topReq,
                               TileMapRequestPacket::tilemap_t,
                               rights )
{

}

TileMapRequest::~TileMapRequest()
{

}

void 
TileMapRequest::getTileMapBuffers( vector<TileMapBufferHolder>& holder ) const
{
#if 1
   // Check the crc by loading maps
   for ( uint32 i = 0; i < m_tileMapBufferHolder.size(); ++i ) {
      
      TileMap tmap;
      BitBuffer tmpBuf( 
            m_tileMapBufferHolder[i].getBuffer()->getBufferAddress(),
            m_tileMapBufferHolder[i].getBuffer()->getBufferSize() );

      tmap.load( tmpBuf,
                 m_mapDesc,
                 m_tileMapBufferHolder[i].getDesc() );

      if ( tmap.getCRC() != m_tileMapBufferHolder[i].getCRC() ) {
         mc2log << error << "[TMR]: CRC MISMATCH loaded: "
                << MC2HEX( tmap.getCRC() ) << " holder: "
                << MC2HEX( m_tileMapBufferHolder[i].getCRC() )
                << endl;
      }
      
      MC2_ASSERT( tmap.getCRC() == m_tileMapBufferHolder[i].getCRC() );
   }
#endif
   holder = m_tileMapBufferHolder;
}


const TileMapBufferHolder& 
TileMapRequest::getTileMapBuffer() const 
{
   MC2_ASSERT( ! m_tileMapBufferHolder.empty() );
   return m_tileMapBufferHolder.front();
}

uint32
TileMapRequest::getEmptyImps() const
{
   uint32 emptyImps = 0;
   for ( uint32 i = 0; i < m_tileMapBufferHolder.size(); ++i ) {
      if ( m_tileMapBufferHolder[ i ].getEmpty() ) {
         TileMapParams tmpParam( m_tileMapBufferHolder[ i ].getDesc() );
         emptyImps |= (1 << tmpParam.getImportanceNbr());
      }
   }
   return emptyImps;
}

void 
TileMapRequest::handleFilteredGfxMapRequestDone()
{
   if ( m_tileMapBufferHolder.empty() ) {
      // Something bad has happened.
      SETSTATE( ERROR );
      return;
   }

   // And we're done
   SETSTATE(DONE_OK);
}
