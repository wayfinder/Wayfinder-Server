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

#include "FilteredGfxMapRequest.h"
#include "TileMapParams.h"

#include "MapSettings.h"
#include "GfxFeatureMapImageRequest.h"
#include "GfxFeatureMapPacket.h"
#include "GfxConstants.h"
#include "TileMap.h"
#include "TileMapUtility.h"
#include "GfxMapFilter.h"
#include "GfxFeatureMap.h"
#include "TopRegionRequest.h"
#include "TileMapBufferHolder.h"

void
FilteredGfxMapRequest::setState(FilteredGfxMapRequest::state_t newState,
                         int line)
{
   //#define PRINT_STATE_CHANGES
#ifdef PRINT_STATE_CHANGES
   if ( newState != m_state ) {
      if ( line < 0 ) {
         mc2dbg << "[TMR]: " 
                << int(m_state) << "->"
                << int(newState) << endl;
      } else {
         mc2dbg << "[TMR]: " << __FILE__ << ":" << line 
                << ' ' <<  int(m_state) << "->"
                << int(newState) << endl;
      }
   } // Don't print if the state doesn't change.
#endif
   m_state = newState;
}

const MapRights& 
FilteredGfxMapRequest::getMapRights() const { 
   return m_mapSettings->getMapRights(); 
}

// To be able to change this into printing the state
// always use SETSTATE instead of m_state=
#define SETSTATE(s) setState(s, __LINE__)

void
FilteredGfxMapRequest::init( const RedLineSettings* redline, 
                             const TileMapParams* params,
                             const MapRights& rights )
{
   // Just in case
   m_status = StringTable::TIMEOUT_ERROR;
   // Set initial state without printing
   m_state = DONE_OK;

   m_gfxMap = NULL;

   // Hardcoded:
   int pixels = m_mapDesc.getNbrPixels( m_param );
   int screenX = pixels;
   int screenY = pixels;
  
   // Create map stuff
   bool highEndDevice = false;
   if ( params != NULL ) {
      highEndDevice = params->getServerPrefix() ==
         STMFDParams::HIGH_END_SERVER_PREFIX;
   }
   m_mapSettings =
      MapSettings::createDefaultTileMapSettings( highEndDevice );

   if ( redline ) {
      m_mapSettings->setRedLineSettings( *redline );
   }  

   bool showMap, showTopo, showPOI, showRoute, showTraffic;
   showMap = showTopo = showPOI = showRoute = showTraffic = false;

   bool showCityCentres = false;

   // Use TileMapUtility to compute the scalelevel.
   int scale = 
      TileMapUtility::getScaleLevel( &m_bbox, screenX, screenY );

   switch ( m_param.getLayer() ) {
      case TileMapTypes::c_mapLayer:
         showCityCentres = true;
         showMap = true;
         //         showPOI = true; // Also needs the citycentres for the map.
         break;
      case TileMapTypes::c_routeLayer:
         if ( m_routeReplyPack != NULL ) {
            showRoute = true;
         }
         break;
      case TileMapTypes::c_poiLayer:
         showPOI = true;
         m_mapSettings->setMapRights( rights );
         break;
      case TileMapTypes::c_trafficLayer: 
         if ( scale < (int)m_mapDesc.getMaxScale( m_param.getLayer() ) ) {
            showTraffic = true;
         }
         break;
      case TileMapTypes::c_acpLayer:
         showPOI = true;
         // also disable any normal pois for this one
         m_mapSettings->setMapRights( rights | 
                                      MapRights::DISABLE_POI_LAYER );
         break;
      case TileMapTypes::c_eventLayer:
         showPOI = true;
         m_mapSettings->addShowMask( MapSettings::EVENT );
         break;
      default :
         break;
   }

   m_mapSettings->setShowCityCentres( showCityCentres );
   m_mapSettings->setMapContent( showMap, showTopo, showPOI, showRoute );
   m_mapSettings->setShowTraffic( showTraffic );
   m_mapSettings->setMaxOneCoordPerPixelForMap( false );
   m_mapSettings->setMaxOneCoordPerPixelForRoute( false );

   if ( params != NULL ) {
      m_mapSettings->setTileMapParamStr( params->getAsString() );
   }

//   float64 meterToPixel = screenY / 
//      (float64) (bbox.getHeight() * GfxConstants::MC2SCALE_TO_METER);

   MC2BoundingBox ccBBox;

   m_gfxFeatureRequest =
      new GfxFeatureMapImageRequest(this,
                                    &m_bbox,
                                    screenX, screenY,
                                    m_param.getLanguageType(),
                                    m_routeReplyPack, // RouteReply
                                    false, // image
                                    false, // exportformat
                                    true, // drawCopyRight
                                    ImageDrawConfig::PNG,
                                    25312, // Size from web
                                    m_mapSettings,
                                    m_topReq,
                                    ccBBox,
                                    scale,
                                    CONTINENT_LEVEL,     // default
                                    scale,
                                    true );              // tilemaps!
                                    
                                                       
   
   SETSTATE(USING_GFX_REQUEST);

   // Enqueue the starting packets from the sub-request.
   processPacket(NULL);
}

FilteredGfxMapRequest::FilteredGfxMapRequest( 
                                const RequestData& requestOrID,
                                const RedLineSettings& redLine,
                                const ServerTileMapFormatDesc& mapDesc,
                                const MC2BoundingBox& bbox,
                                int tileMapLayerID,
                                int tileMapImportanceNbr,
                                int pixelToMeter,
                                const RouteReplyPacket* routeReplyPack,
                                const TopRegionRequest* topReq,
                                bool removeNames,
                                TileMapRequestPacket::resultMap_t resultMapType,
                                const MapRights& rights )

      : RequestWithStatus( requestOrID ) , 
        m_routeReplyPack( routeReplyPack ),
        m_mapDesc( mapDesc ),
        m_removeNames( removeNames ),
        m_bbox( bbox ),
        m_topReq( topReq ),
        m_resultMapType( resultMapType )
{

   // Make sure we don't divide by zero.
   m_pixelToMeter = MAX( pixelToMeter, 1 );

   int detailLevel = 
      mapDesc.getDetailLevelFromLayerID( tileMapLayerID, pixelToMeter );
   
   RouteID dont_complain;
   m_param.setParams( 0, // Server prefix, unused
                      false, // gzip, unused
                      tileMapLayerID,
                      TileMapTypes::tileMapData, // unused
                      tileMapImportanceNbr,  // Importance number.
                      LangTypes::english, // unused
                      0, // lat index, unused
                      0, // lon index, unused
                      detailLevel,
                      &dont_complain ); // route id, unused.

   init( &redLine, NULL, rights );
}

FilteredGfxMapRequest::
FilteredGfxMapRequest( const RequestData& requestOrID,
                       const ServerTileMapFormatDesc& mapDesc,
                       const TileMapParams& params,
                       const RouteReplyPacket* routeReplyPack,
                       const TopRegionRequest* topReq,
                       TileMapRequestPacket::resultMap_t resultMapType,
                       const MapRights& rights )
      : RequestWithStatus( requestOrID ) , 
        m_param( params ),
        m_routeReplyPack( routeReplyPack ),
        m_mapDesc( mapDesc ),
        m_removeNames( false ),
        m_topReq( topReq ),
        m_resultMapType( resultMapType ),
        m_pixelToMeter( - 1 )
{
   // Set bbox from tilemap param.
   m_mapDesc.getBBoxFromTileIndex( m_param.getLayer(),
                                   m_bbox, 
                                   m_param.getDetailLevel(),
                                   m_param.getTileIndexLat(),
                                   m_param.getTileIndexLon() );
   
   init( NULL, &params, rights );
}

FilteredGfxMapRequest::~FilteredGfxMapRequest()
{
   delete m_gfxFeatureRequest;
   delete m_mapSettings;
   delete m_gfxMap;

   for ( uint32 i = 0; i < m_tileMapBufferHolder.size(); ++i ) {
      m_tileMapBufferHolder[ i ].deleteBuffer();
   }
   for ( uint32 i = 0; i < m_packetContainersToDelete.size(); ++i ) {
      delete m_packetContainersToDelete[ i ];
   }
}

StringTable::stringCode
FilteredGfxMapRequest::getStatus() const
{
   return m_status;
}

bool
FilteredGfxMapRequest::requestDone()
{
   mc2dbg8 << "========== FilteredGfxMapRequest: requestDone? " << endl;
   mc2dbg8 << ": " << (bool)(m_state == DONE_OK || m_state == ERROR) << endl;
   return m_state == DONE_OK || m_state == ERROR;
}

void
FilteredGfxMapRequest::processPacket( PacketContainer* pack )
{
   // NB! pack can be NULL!
   switch ( m_state ) {
      case USING_GFX_REQUEST:
         mc2dbg8 << "================= Fetching gfx." << endl;
         if ( processSubRequestPacket(m_gfxFeatureRequest,
                                      pack,
                                      m_status) ) {
            // Other request done
            if ( m_status == StringTable::OK ) {
               handleGfxRequestDone();
            } else {
               SETSTATE(ERROR);
            }
         } else {
            // Just keep running.
         }
         break;
      
      case GENERIC_STATE: {
         mc2dbg8 << "================= Generic state." << endl;
         // Only TileMapReplyPackets should arrive.
         
         TileMapReplyPacket* reply = 
            static_cast<TileMapReplyPacket*>(pack->getPacket());
         if ( reply->getStatus() != StringTable::OK ) {
            m_status = StringTable::stringCode( reply->getStatus() );
            delete pack;
            SETSTATE( ERROR );
            return;
         }
         // else, everything is fine.
         m_status = StringTable::OK;
            
         // Get the tilemap buffers.
         // Appends holders to the buffer.
         reply->get( m_tileMapBufferHolder );

         // Need to keep the packetcontainer until this object is
         // deleted.
         m_packetContainersToDelete.push_back( pack );
         
         handleFilteredGfxMapRequestDone();
         
         } break;
      default:
         mc2log << warn << "[TMR]: Got packet in state = "
                << uint32(m_state) << endl;
   }
}
   
const GfxFeatureMap*
FilteredGfxMapRequest::getGfxFeatureMap() const
{
   return m_gfxMap;
}

void
FilteredGfxMapRequest::handleGfxRequestDone()
{
   // We are not yet done.
   m_status = StringTable::TIMEOUT_ERROR;
   GFMDataHolder* holder = m_gfxFeatureRequest->getGFMDataHolder();

   MC2_ASSERT( holder != NULL );

   SETSTATE( GENERIC_STATE );

   TileMapRequestPacket* packet = new TileMapRequestPacket( 
                  *holder,
                  m_mapDesc.getSTMFDParams(),
                  m_removeNames,
                  m_resultMapType,
                  m_param,
                  m_pixelToMeter );

   enqueuePacket( updateIDs( packet ), MODULE_TYPE_TILE );
}

void 
FilteredGfxMapRequest::handleFilteredGfxMapRequestDone()
{
   m_gfxMap = new GfxFeatureMap();
   // Convert to DataBuffer so it can be loaded.
   DataBuffer tmpBuf( 
         m_tileMapBufferHolder.front().getBuffer()->getBufferAddress(),
         m_tileMapBufferHolder.front().getBuffer()->getBufferSize() );
   m_gfxMap->load( &tmpBuf );
   
   // And we're done
   SETSTATE(DONE_OK);
}

