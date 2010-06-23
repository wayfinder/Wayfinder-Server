/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxProcessor.h"

#include "ISABThread.h"
#include "MapSettings.h"
#include "GfxFeatureMapImagePacket.h"
#include "Packet.h"
#include "DataBuffer.h"
#include "MapDrawer.h"
#include "ImageDraw.h"
#include "GfxFeatureMap.h"
#include "ScopedArray.h"
#include "FilePtr.h"

#include "POIImageIdentificationTable.h"
#include "ServerTileMapFormatDesc.h"
#include "MapDrawerExport.h"

#include <memory>

GfxProcessor::GfxProcessor( MapSafeVector* loadedMaps,
                            const char* packetFileName ) : 
   Processor( loadedMaps ),
   m_packetFileName( packetFileName ? packetFileName : "" ),
   m_stmfd( new 
            ServerTileMapFormatDesc( STMFDParams( LangTypes::english, 
                                                  false ) ) ),
   m_poiImageTable( NULL )
{
   m_stmfd->setData();
   m_poiImageTable = new POIImageIdentificationTable( *m_stmfd );
}

GfxProcessor::~GfxProcessor() {
   delete m_poiImageTable;
   delete m_stmfd;
}

GfxFeatureMapImageReplyPacket* 
GfxProcessor::
processGfxFeatureMapImageRequest( const GfxFeatureMapImageRequestPacket* p)
{       
   mc2dbg4 << "[GfxProcessor]" << __FUNCTION__
           << " packet length: "<< p->getLength() << endl;
   MapSettings mapSettings;
   auto_ptr<DataBuffer> data( p->getGfxFeatureMapData( &mapSettings ) );
   GfxFeatureMap gfxMap;
   gfxMap.load( data.get() );
   
   auto_ptr<GfxFeatureMapImageReplyPacket> reply;

   if ( gfxMap.getScreenX() > 10000 || gfxMap.getScreenY() > 10000 ) {
      // Screen size too large, you probably whants something like
      mc2log << error << "GM:processGfxFeatureMapImageRequest image "
             << "size too large " <<  gfxMap.getScreenX() << "x" 
             << gfxMap.getScreenY() << " changing to 200x200" << endl;
      gfxMap.setScreenSize( 200, 200 );
   }
   
   if( !p->getExportFormat() ) {
      ImageDrawConfig::imageFormat format = p->getImageFormat();
      uint16 size = p->getSizeParameter();
      MapDrawer mapDraw;
      uint32 imageSize = 0;

      gfxMap.recalculateLevelAndSplitRoads();
      ScopedArray<byte> 
         imageBuff( mapDraw.
                    drawGfxFeatureMap( &gfxMap,
                                       imageSize,
                                       &mapSettings,
                                       p->getMapRotation(),
                                       p->getArrowType(),
                                       p->getBeforeturn(), 
                                       p->getAfterturn(),
                                       30, 6,  // arrow angle, arrow length
                                       static_cast<GDUtils::Color::CoolColor>
                                       ( 0x000000 ), // arrow color
                                       (size > 4096), // if draw text
                                       format, // format
                                       p->getCopyright(), // copyright str
                                       // if draw copyright
                                       p->getDrawCopyRight(),
                                       m_poiImageTable ) );
                    

      reply.reset( new GfxFeatureMapImageReplyPacket(imageSize, p, true) );
     
      reply->setImageData(imageSize, imageBuff.get());
     
   } else {

     mc2dbg8 << "MaxScaleLevel: "
             << (uint32)p->getMaxScaleLevel()
             << " MinScaleLevel: " 
             << (uint32)p->getMinScaleLevel()
             << endl;      

      //This indicates that the ExportScalableFeatureMapdata was asked for.
     if ( p->getScalableExportFormat() ){
         mc2dbg8 << "GfxProcessor::ExportScalableFormat" << endl;
         auto_ptr<DataBuffer> exportScalableMapData(
           MapDrawerExport::makeExportScalableGfxFeatureMap( &gfxMap,
                                                             &mapSettings, 
                                                             false ) );
         reply.reset(new GfxFeatureMapImageReplyPacket(
                                    exportScalableMapData->getCurrentOffset(),
                                    p,
                                    false ) );
         
         reply->setGfxExportScalableFeatureMapData(
                              exportScalableMapData->getCurrentOffset(),
                              exportScalableMapData.get() );

         mc2dbg1 << "GfxProcessor: map size = " 
                 << exportScalableMapData->getCurrentOffset()
                 << "bytes." << endl;

      } else { //ExportFeatureMapdata was asked for.
         auto_ptr<DataBuffer> exportMapData(
            MapDrawerExport::
            makeExportGfxFeatureMap( &gfxMap,
                                     &mapSettings,
                                     false,
                                     mapSettings.getDrawingProjection() ) );

         reply.reset( new GfxFeatureMapImageReplyPacket(
                                            exportMapData->getBufferSize(),
                                            p,
                                            false) );
      
         reply->setGfxExportFeatureMapData(
               exportMapData->getCurrentOffset(),
               exportMapData.get() );
     

      }
   }
   
   return reply.release();
}


Packet* GfxProcessor::handleRequestPacket( const RequestPacket& p,
                                           char* packetInfo)
{

   Packet *replyPacket = NULL;

   // Save packet to file... Maybe.
   // FIXME: Move this to Processor or JT.
   if ( ! m_packetFileName.empty() ) {
      const RequestPacket* requestPacket = &p;
      // Use this code to write the packets to disk for later profiling
      FileUtils::FilePtr packetFile( fopen(m_packetFileName.c_str(), "ab") );
      if ( packetFile.get() != NULL ) {
         uint32 packLen = ntohl(requestPacket->getLength());
         fwrite(&packLen, 4, 1, packetFile.get() );
         fwrite(requestPacket->getBuf(),
                requestPacket->getLength(), 1, packetFile.get());
      }
   }

   uint16 subType = p.getSubType();
   switch (subType) {     
   case Packet::PACKETTYPE_GFXFEATUREMAP_IMAGE_REQUEST :
      mc2dbg << "Processing the image req." << endl;
      
      replyPacket = processGfxFeatureMapImageRequest( 
                   static_cast<const GfxFeatureMapImageRequestPacket*> (&p));
      break; 
   default:
      mc2log << error << "[GP]: Got unkown packet" << endl;
      break;
   }
   
   return replyPacket;
}


int GfxProcessor::getCurrentStatus()
{
   return 0;
}






