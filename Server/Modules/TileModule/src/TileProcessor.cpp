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

#include "TileProcessor.h"
#include "Packet.h"
#include "TileMapPacket.h"
#include "ServerTileMapFormatDesc.h"
#include "GFMDataHolder.h"
#include "GfxMapFilter.h"
#include "GfxConstants.h"
#include "TileMapBufferHolder.h"
#include "TileMapCreator.h"

#include "TileMapPOIPacket.h"
#include "POISetProperties.h"

#include "DeleteHelpers.h"
#include "MC2CRC32.h"

#include <memory>
#include <sstream>

TileProcessor::TileProcessor(MapSafeVector* loadedMaps ) 
      : Processor(loadedMaps) 
{
   const ServerTileMapFormatDesc* desc =
      getTileMapFormatDesc( STMFDParams( LangTypes::english, false ) );
   m_tileMapCreator.reset( new TileMapCreator( *desc ) );
}

TileProcessor::~TileProcessor()
{
   STLUtility::deleteAllSecond( m_stmfds );
}

Packet*
TileProcessor::handleRequestPacket( const RequestPacket& request,
                                    char* packetInfo )
{
   ReplyPacket* reply = NULL;

   mc2dbg4 << "TP::handleRequest() going to handle packet" << endl;

   switch ( request.getSubType() ) {
      case Packet::PACKETTYPE_TILEMAP_REQUEST: {
         const TileMapRequestPacket* tileReq =
            static_cast<const TileMapRequestPacket*>(&request);

         reply = handleTileMapRequest( tileReq, packetInfo ); 
      }
      break;
   case Packet::PACKETTYPE_TILEMAPPOI_REQUEST:
      reply = 
         handleTileMapPOIRequest( static_cast< const TileMapPOIRequestPacket& >
                                  ( request ),
                                  packetInfo );
      break;
     default:
        mc2log << warn << "[TP]: Unknown packet "
               << request.getSubTypeAsString() << endl;
        break;
   }
   
   return reply;
}

int TileProcessor::getCurrentStatus()
{
   return 0;
}

const ServerTileMapFormatDesc*
TileProcessor::getTileMapFormatDesc( const STMFDParams& settings )
{
   map<STMFDParams, ServerTileMapFormatDesc*>::const_iterator findIt =
      m_stmfds.find( settings );
   if ( findIt != m_stmfds.end() ) {
      return findIt->second;
   } else {
      ServerTileMapFormatDesc* stmfd = 
         new ServerTileMapFormatDesc( settings );
      stmfd->setData();
      m_stmfds[ settings ] = stmfd;
      return stmfd;
   } 
}
   

TileMapReplyPacket*
TileProcessor::
handleTileMapRequest( const TileMapRequestPacket* tileReq,
                      char* packetInfo )
{

   GFMDataHolder gfmData;
   STMFDParams stmfdParams;
   bool removeNames;
   TileMapRequestPacket::resultMap_t resultMapType;
   MC2SimpleString param;
   int pixelToMeter; 
   
   tileReq->get( gfmData, 
                 stmfdParams, 
                 removeNames,
                 resultMapType,
                 param,
                 pixelToMeter );

   const ServerTileMapFormatDesc* stmfd = 
      getTileMapFormatDesc( stmfdParams );

   DataBuffer* gfxBuf = gfmData.getBuffer();
 
   // Find out which params that should be extracted.
   vector<MC2SimpleString> params;

   if ( resultMapType == TileMapRequestPacket::tilemap_t ) {
      // TileMap. Extract all importances.
      stmfd->getAllImportances( param, params );
      
   } else if ( resultMapType == TileMapRequestPacket::gfxmap_t ) {
      // GfxMap. Only extract this param.
      params.push_back( param );
   } else {
      // Unsupported resultMapType.
      // Return.
      sprintf( packetInfo, 
              "Unsupported resultMap_t %d", (int) resultMapType );
      return new TileMapReplyPacket( tileReq, StringTable::NOTOK );
   }
   
   vector<TileMapBufferHolder> result;
   result.reserve( params.size() );
   
   
   // Set bbox from tilemap param.
   TileMapParams tmpParam( param );
   MC2BoundingBox bbox;
   stmfd->getBBoxFromTileIndex( tmpParam.getLayer(),
         bbox, 
         tmpParam.getDetailLevel(),
         tmpParam.getTileIndexLat(),
         tmpParam.getTileIndexLon() );

   float64 meterToPixelFactor = 0;
   if ( pixelToMeter >= 0 ) {
      // Use the pixelToMeter value from the packet.
      meterToPixelFactor = 1 / float64( pixelToMeter );
   } else {
      // Calculate the meterToPixelFactor using the param.
      meterToPixelFactor = stmfd->getNbrPixels( tmpParam ) / 
         (float64) (bbox.getHeight() * GfxConstants::MC2SCALE_TO_METER);
   }

   // The empty importances.
   uint32 emptyImportances = 0;
  
   vector<TileMap*> tmpTileMaps;
   
   for ( uint32 i = 0; i < params.size(); ++i ) {
   
      TileMapParams curParam( params[ i ] );
      
      // Get the full unfiltered gfxmap.
      GfxFeatureMap gfxMap;
      gfxMap.load( gfxBuf );
      
      // Filter the gfxmap. Remove unwanted features.
      GfxFeatureMap* filteredGfxMap = 
         GfxMapFilter::filterGfxMap( gfxMap, 
                                     *stmfd,
                                     curParam,
                                     meterToPixelFactor,
                                     // Adopt to lowest zoom level, this detail
                                     // level is used for instead of highest.
                                     removeNames ); 

      

     
      TileMapBufferHolder tmBufHolder;
      if ( resultMapType == TileMapRequestPacket::gfxmap_t ) {
         // The result should be a gfxfeaturemap.
         tmBufHolder.m_empty = filteredGfxMap->getNbrFeatures() == 0;
         DataBuffer tmpBuf( filteredGfxMap->getMapSize() );
         filteredGfxMap->save( &tmpBuf );
         // XXX: Copies the DataBuffer to a BitBuffer.
         tmBufHolder.m_buf = new BitBuffer( tmpBuf.getBufferSize() );
         tmBufHolder.m_buf->writeNextByteArray( tmpBuf.getBufferAddress(),
               tmpBuf.getBufferSize() );
         // The crc and empty attributes are always invalid for gfxmaps,
         // i.e. they can be trusted since they do not matter anyway.
         tmBufHolder.m_trustAttributes = true;

      } else if ( resultMapType == TileMapRequestPacket::tilemap_t ) {
         // The result should be a tilemap.

         // Create a tilemap from the gfxmap.
         const TileImportanceNotice* importance = 
            stmfd->getImportanceNbr( curParam );

         // XXX Dump the param and the tilemap.
         DEBUG8(
                mc2dbg << "Dumping[" << curParam.getAsString() << "]:" 
                << curParam << endl;
                filteredGfxMap->dump( 10 );
         );

         // Store the TileMap object. Need to keep all TileMaps before
         // the empty importances can be counted, and the TileMaps can be
         // written as a buffer.
         TileMap* tileMap = m_tileMapCreator->
            createTileMap( curParam,
                           stmfd,
                           *importance,
                           filteredGfxMap );

         tmpTileMaps.push_back( tileMap );

         // Update the empty importances.
         if ( tileMap->empty() ) {
            emptyImportances |= (1 << curParam.getImportanceNbr() );
         }

      } // else: should not happen. Already checked this above.
      tmBufHolder.m_desc = params[ i ].c_str();
          
      result.push_back( tmBufHolder ); 
      // Delete the temporary filtered gfxmap.
      delete filteredGfxMap;
   }


   if ( resultMapType == TileMapRequestPacket::tilemap_t ) {
      MC2_ASSERT( tmpTileMaps.size() == result.size() );
      for ( uint32 i = 0; i < result.size(); ++i ) {
         TileMapBufferHolder& tmBufHolder = result[ i ];
         
         TileMap* tileMap = tmpTileMaps[ i ];
         MC2_ASSERT( tileMap != NULL );

         // Update the empty importances.
         tileMap->setEmptyImportances( emptyImportances );

         // Store the tilemap as a buffer.
         BitBuffer* buf = new BitBuffer(1024*1024*10);
         tileMap->save( *buf, &tmBufHolder.m_crc );

         // The map buffer cannot be empty.
         MC2_ASSERT( buf->getCurrentOffset() != 0 );
         
         tmBufHolder.m_buf = buf;

         tmBufHolder.m_empty = tileMap->empty();
         // The crc and empty attributes are set correctly.
         tmBufHolder.m_trustAttributes = true;
         delete tileMap;
      }
   }

   TileMapReplyPacket* reply = new TileMapReplyPacket( tileReq,
                                                       result );

   // Delete 
   for ( uint32 i = 0; i < result.size(); ++i ) {
      result[ i ].deleteBuffer();
   }

   sprintf( packetInfo, "%s", param.c_str() );
   return reply;
   
}


ReplyPacket* TileProcessor::
handleTileMapPOIRequest( const TileMapPOIRequestPacket& packet,
                         char* packetInfo ) const {

   TileMapPOIRequestPacket::ACPData data;
   packet.getData( data );
   const TileMapFormatDesc* desc = 
      const_cast<TileProcessor*>(this)->
      getTileMapFormatDesc( STMFDParams( LangTypes::english,
                                         false ) );

   if ( data.empty() ||
        data[ 0 ].second.empty() ) {
      // return empty answer
      TileMapBufferHolderVector vec;
      return new TileMapPOIReplyPacket( packet, StringTable::OK,
                                        vec );
   }

   // the result tilemap
   vector<WritableTileMap*> finalTiles;

   // TODO: sort data vector according to priority

   vector<TileMap*> otherTiles;
   vector<uint32> crcs;
   // for all poi sets, merge with the tilemap
   for ( uint32 i = 0; i < data.size(); ++i ) {
      //      const POISet& poi = data[ i ].first;
      const TileMapBufferHolderVector& holders = data[ i ].second;
      // setup 
      if ( finalTiles.size() < holders.size() ) {
         // Note: this should only happen for the first tile
         finalTiles.reserve( holders.size() );
      }
      for ( uint32 holderIdx = 0; holderIdx < holders.size(); ++holderIdx ) {

         const char* paramStr = holders[ holderIdx ].getDesc();
         mc2dbg << "ACP: Merging: " << paramStr << endl;
         mc2dbg << "ACP: Buffer size: " 
                << holders[ holderIdx ].getBuffer()->getBufferSize()
                << endl;

         TileMapParams inParam( paramStr );

         TileMap* otherTile = new TileMap();
         if ( holders[ holderIdx ].getBuffer()->getBufferSize() != 0 ) {
            otherTile->load( *holders[ holderIdx ].getBuffer(), 
                             *desc, paramStr );
            
         }


         otherTiles.push_back( otherTile );

         // if we haven't created the tilemap for this importance level, then;
         // Do it!(tm)
         if ( finalTiles.size() < holderIdx + 1) {
            // add new tilemap
            finalTiles.
               push_back( new WritableTileMap( inParam, *desc,
                                               desc->
                                               getImportanceNbr( inParam )->
                                               getType(),
                                               0 ));
            
         }

         if ( holders[ holderIdx ].getBuffer() == 0 ||
              holders[ holderIdx ].getBuffer()->getBufferSize() == 0 ) {
            continue;
         } else {
            crcs.push_back( holders[ holderIdx ].getCRC() );
         }
         // merge the other tile into final tile map
         finalTiles[ holderIdx ]->merge( *otherTile );
      }
      
   }

   // lets compose a new crc from all the other crc 
   uint32 finalCRC = MC2CRC32::crc32( reinterpret_cast<byte*>( &crcs[ 0 ] ),
                                      crcs.size() * sizeof ( uint32 ) );
   if ( finalTiles.empty() ) {
      // setup final tile, empty
      /*
      finalTile.reset( new WritableTileMap( inParam, *desc,
                                            desc->getImportanceNbr( inParam )->getType(),
                                            0 ) );
      */
   }



   TileMapBufferHolderVector returnHolders;
   
   
   for ( uint32 tileIdx = 0; tileIdx < finalTiles.size(); ++tileIdx ) {

      WritableTileMap& finalTile = *finalTiles[ tileIdx ];
      // force crc 
      BitBuffer* buf = new BitBuffer( 1024*1024*10 ); // 10Mbytes   
      finalTile.save( *buf, NULL, finalCRC );
      
      // holder now owns buf
      TileMapBufferHolder holder( finalTile.getParam()->getAsString().c_str(), 
                                  buf,
                                  finalCRC, false );

      returnHolders.push_back( holder );
   }

   TileMapPOIReplyPacket* reply = 
      new TileMapPOIReplyPacket( packet, StringTable::OK,
                                 returnHolders );

   STLUtility::deleteValues( otherTiles );
   STLUtility::deleteValues( finalTiles );


   return reply;

}
