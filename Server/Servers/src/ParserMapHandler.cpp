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

#include "ParserMapHandler.h"
#include "BitBuffer.h"
#include "MC2Point.h"

#include "DrawingProjection.h"
#include "GFMDataHolder.h"
#include "GfxFeatureMap.h"
#include "GfxFeatureMapImageRequest.h"
#include "MapSettings.h"
#include "NameUtility.h"
#include "ParserThread.h"
#include "ParserThreadGroup.h"
#include "ServerTileMapFormatDesc.h"
#include "SimplePoiDesc.h"
#include "SimplePoiMap.h"
#include "POIImageIdentificationTable.h"
#include "UserData.h"
#include "ClientSettings.h"

#include <memory>
#include <iostream>

ParserMapHandler::ParserMapHandler( ParserThread* thread,
                                    ParserThreadGroup* group )
      : ParserHandler( thread, group )
{
}

ParserMapHandler::~ParserMapHandler()
{
}

GfxFeatureMapImageRequest*
ParserMapHandler::createPoiGfxRequest( const SquareMapParams& params )
                                    
{
   mc2dbg << params << endl;
   auto_ptr<MercatorProjection> projection (
      new MercatorProjection( params.x(),
                              params.y(),
                              params.zoom(),
                              params.nbrPixels() ) );
   projection->init();

   if ( projection->getStatus() != StringTable::OK ) {
      return NULL;
   }

   MapSettings* mapSettings = new MapSettings();
   
   mapSettings->setMapContent( false,  // map
                               false,  // topgraph map
                               true,  // poi
                               false ); // route
   mapSettings->setDrawScale( false );
   mapSettings->setShowTraffic( false );
   mapSettings->setDrawingProjection( projection.get() );
   // setup map right 
   if ( m_thread->getCurrentUser() ) {
      MapRights userMapRights =
         m_thread->getCurrentUser()->getUser()->
         getAllMapRights( TimeUtility::getRealTime() );
      mapSettings->setMapRights( userMapRights );
   }

   const ClientSetting* clientSetting = m_thread->getClientSetting();
   if ( clientSetting != NULL ) {
      mapSettings->setImageSet( clientSetting->getImageSet() );
   }
   
   GfxFeatureMapImageRequest* gfxRequest =
      new GfxFeatureMapImageRequest( m_thread->getNextRequestID(),
                                     &projection->getLargerBoundingBox(),
                                     params.nbrPixels(),
                                     params.nbrPixels(),
                                     params.lang(),
                                     NULL,
                                     false, // image
                                     false,
                                     false,
                                     ImageDrawConfig::PNG,
                                     17971,
                                     mapSettings,
                                     m_thread->getTopRegionRequest(),
                                     MC2BoundingBox(),
                                     projection->getScaleLevel() );
   // Set the user (may be NULL)
   gfxRequest->setUser( m_thread->getCurrentUser() );
   // Deleted by MapSettings.
   projection.release();
   // Make the gfxRequest keep the mapSettings.
   gfxRequest->ownMapSettings();
   return gfxRequest;
}

ParserMapHandler::bufPair_t
ParserMapHandler::
getSimplePoiMap( const SquareMapParams& params )
{
   // Create request
   auto_ptr<GfxFeatureMapImageRequest> 
      gfxReq( createPoiGfxRequest( params ) );
   
   if ( gfxReq.get() == NULL ) {
      // Bad params
      return bufPair_t(BAD_PARAMS, NULL);
   }

   // Run the request
   m_thread->putRequest( gfxReq.get() );

   if ( gfxReq->getStatus() != StringTable::OK ) {
      // Failure, probably timeout.
      return bufPair_t(TIMEOUT, NULL);
   }

   // Get the map
   GFMDataHolder* gmh = gfxReq->getGFMDataHolder();
   MC2_ASSERT( gmh != NULL );   
   const GfxFeatureMap* gfxmap = gmh->getGfxFeatureMap();
   MC2_ASSERT( gfxmap != NULL );

   // Map done

   // Create projection ( again )
   MercatorProjection proj( params.x(),
                            params.y(),
                            params.zoom(),
                            params.nbrPixels() );
   proj.init();

   const ServerTileMapFormatDesc& desc = 
      *m_group->getTileMapFormatDesc( STMFDParams( params.lang(), 
                                                   false ),// night mode
                                      m_thread );
   const POIImageIdentificationTable& imageTable = m_group->getPOIImageIdTable();
   // Create the poi map so that we can send it.
   SimplePoiMap spm( *gfxmap, desc, proj, imageTable );
   
   return bufPair_t( OK, spm.getAsNewBytes() );
}

ParserMapHandler::bufPair_t
ParserMapHandler::getSimplePoiDesc( const SimplePoiDescParams& params,
                                    ImageTable::ImageSet imageSet )
{
   const ServerTileMapFormatDesc& serverDesc = 
      *m_group->
      getTileMapFormatDesc( STMFDParams( LangTypes::english, 
                                         false, // night mode
                                         STMFDParams::DEFAULT_LAYERS,
                                         0, // drawSettingVersion
                                         imageSet ), 
                            m_thread );
   SimplePoiDesc desc ( serverDesc,
                        "/TMap/", // Base URL for url-based clients.
                        m_group->getPOIImageIdTable() );
   return bufPair_t(OK, desc.getAsNewBytes());
}

ostream& operator << ( ostream& o, const SquareMapParams& p ) {
   return o << "[SQMP]: (x,y,zoom,lang,pixels) = ("
            << p.x() << "," << p.y() << "," << p.zoom() << ","
            << LangTypes::getLanguageAsISO639( p.lang() ) << ","
            << p.nbrPixels() << ")";
}
