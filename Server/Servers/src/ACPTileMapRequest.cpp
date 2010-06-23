/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ACPTileMapRequest.h"

#include "MultiRequest.h"
#include "STLStringUtility.h"
#include "POISetProperties.h"
#include "ParserThread.h"
#include "TileMapPOIPacket.h"
#include "DeleteHelpers.h"

ACPRequestHolder::ACPRequestHolder( const TileMapRequest* request):
   m_params( request->getTileMapParams() ) {
   request->getTileMapBuffers( m_buffers );
}

ACPTileMapRequest::ACPTileMapRequest( const RequestData& reqOrID,
                                      const ServerTileMapFormatDesc& mapDesc,
                                      const TileMapParams& params,
                                      const TopRegionRequest* topReq,
                                      const MapRights& rights,
                                      const RequestVector& doneRequests,
                                      RightsVector& doneRights  ):
   TileMapRequest( reqOrID,
                   mapDesc, params,
                   NULL, // not interested in route
                   topReq, rights ),
   m_answer( NULL ) {

   m_rights.swap( doneRights );

   for ( uint32 i = 0; i < doneRequests.size(); ++i ) {
      m_tilerequests.
         push_back( new ACPRequestHolder( doneRequests[ i ] ) );
   }
   // compose real request
   composeRequest( reqOrID );
}

ACPTileMapRequest::ACPTileMapRequest( const RequestData& reqOrID,
                                      const ServerTileMapFormatDesc& mapDesc,
                                      const TileMapParams& params,
                                      const TopRegionRequest* topReq,
                                      const MapRights& rights,
                                      RequestInfoVector& doneRequests,
                                      RightsVector& doneRights  ):
   TileMapRequest( reqOrID,
                   mapDesc, params,
                   NULL, // not interested in route
                   topReq, rights ),
   m_answer( NULL ) {
   
   m_tilerequests.swap( doneRequests );
   m_rights.swap( doneRights );

   // compose real request
   composeRequest( reqOrID );
}

void ACPTileMapRequest::composeRequest( const RequestData& reqOrID ) {
   // setup tile map data
   TileMapPOIRequestPacket::ACPCont data;
   for ( uint32 i = 0; i < m_tilerequests.size(); ++i ) {
      data.push_back( make_pair( m_rights[ i ], 
                                 &m_tilerequests[ i ]->
                                 getTileMapBufferHolders() ) );
   }

   Packet* pack = new TileMapPOIRequestPacket( data );
   pack->setRequestID( reqOrID.getID() );
   pack->setPacketID( getNextPacketID() );
   m_request = new PacketContainer( pack,
                                    0, 0,
                                    MODULE_TYPE_TILE );
}

ACPTileMapRequest::~ACPTileMapRequest() {
   STLUtility::deleteValues( m_tilerequests );
   delete m_answer;
}

PacketContainer* ACPTileMapRequest::getNextPacket() {
   if ( m_done ) {
      return NULL;
   }

   PacketContainer* pack = m_request;
   m_request = NULL;
   return pack;
}

PacketContainer* ACPTileMapRequest::getAnswer() {
   return m_answer;
}

void ACPTileMapRequest::processPacket( PacketContainer* pack ) {
   m_done = true;
   m_answer = pack;
   m_status = 
      StringTable::
      stringCode( static_cast<TileMapPOIReplyPacket*>
                  ( pack->getPacket() )->getStatus() );
}

void ACPTileMapRequest::
getTileMapBuffers( vector<TileMapBufferHolder>& holder ) const {
   if ( m_answer &&
        m_answer->getPacket() &&
        ! m_tilerequests.empty() ) {
      static_cast<TileMapPOIReplyPacket*>( m_answer->getPacket() )->
         getData( holder );
   } else {
      mc2log << fatal << "[ACPTileMapRequest] No Answer!" << endl;
      mc2log << fatal << "[ACPTileMapRequest] m_answer = " << m_answer << endl;
   }
}

void ACPTileMapRequest::
getCacheNames( const TileMapParams& inParam, const MapRights& rights,
               CacheStrings& cacheInfos ) {
   // Add tilemap string params as: TILEMAP.MAPRIGHT
   // For example: T+boom.64
   MC2String prefixTileString = inParam.getAsString().c_str();
   prefixTileString += ".";

   typedef POISetProperties::POISetMap::const_iterator const_iterator;
   const_iterator it = POISetProperties::getAllPOISets().begin();
   const_iterator itEnd = POISetProperties::getAllPOISets().end();
   for ( ; it != itEnd; ++it ) {
      if ( rights & MapRights( it->getMapRight() ) ) {
         cacheInfos.push_back( make_pair( ( prefixTileString + 
                                            STLStringUtility::
                                            uint2str( 1 << it->getMapRight() ) ).c_str(),
                                          it->getMapRight() ) );
      }
   }
}

void ACPTileMapRequest::getTileMapRequests( ParamRequests& requests ) const {
   // for each tile map request, create (param, request) pair and put in 
   // return vector
   requests.resize( m_tilerequests.size() );
   for ( uint32 i = 0; i < m_tilerequests.size(); ++i ) {
      // create param string
      MC2String paramStr = 
         m_tilerequests[ i ]->getTileMapParams().getAsString().c_str();
      paramStr += "." + STLStringUtility::uint2str( 1 << m_rights[ i ] );
      // setup param string + request
      requests[ i ].first = paramStr;
      requests[ i ].second = &m_tilerequests[ i ]->getTileMapBufferHolders();
   }
}

bool ACPTileMapRequest::isACPParam( const TileMapParams& inParam ) {
   return inParam.getLayer() == TileMapTypes::c_acpLayer;
}



void ACPTileMapRequest::
createTileMapRequests( ParserThread& thread,
                       const ServerTileMapFormatDesc& mapDesc,
                       const TileMapParams& params,
                       const TopRegionRequest* topReq,
                       const MapRights& rights,
                       vector<TileMapRequest*>& requests,
                       vector<MapRights::Rights>& rightsVector ) {
    // setup requests for each specific right
   typedef POISetProperties::POISetMap::const_iterator const_iterator;
   const_iterator it = POISetProperties::getAllPOISets().begin();
   const_iterator itEnd = POISetProperties::getAllPOISets().end();
   for ( ; it != itEnd; ++it ) {
      if ( rights & MapRights( it->getMapRight() ) ) {
         mc2dbg << "[ACPTileMapRequest] ACP tiles: " 
                << ( rights & MapRights( it->getMapRight() ) )
                << endl;
         // store mapright::rights enum as we need it later when
         // storing specific right in cache.
         rightsVector.push_back( it->getMapRight() );
         // create the request
         requests.
            push_back( new TileMapRequest( thread.getNextRequestID(),
                                           mapDesc,
                                           params,
                                           NULL, // not interested in route
                                           topReq,
                                           it->getMapRight() ) );
      }
   }
}
