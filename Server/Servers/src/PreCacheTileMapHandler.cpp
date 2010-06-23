/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PreCacheTileMapHandler.h"

#include "RouteID.h"
#include "MC2Coordinate.h"
#include "ServerTileMapFormatDesc.h"
#include "ParserThreadGroup.h"
#include "ExpandedRoute.h"
#include "ExpandedRouteRoadItem.h"
#include "PreCacheRouteData.h"
#include "TileMapQuery.h"
#include "Properties.h"
#include "NullTileMapQuery.h"

#include <memory>
#include <iterator>
namespace {

/// determines scale from speed in kilometer per hour.
uint32 getScaleFromSpeedKph( uint32 speedKph ) {
   if ( speedKph < 30 ) {
      return 2;
   } else if ( speedKph < 55 ) {
      return 4;
   } else if ( speedKph < 75 ) {
      return 10;
   } else if ( speedKph < 95 ) {
      return 20;
   } else {
      return 40;
   }
}

}

PreCacheTileMapHandler::
PreCacheTileMapHandler( ParserThreadGroupHandle group,
                        RouteQueue& routeDataQueue ):
   ParserThread( group, "PreCacheTileMapHandler" ),
   m_routeData( routeDataQueue ) {

}

PreCacheTileMapHandler::~PreCacheTileMapHandler() {
   mc2dbg << "[PreCacheTileMapHandler] terminated." << endl;
}

void PreCacheTileMapHandler::run() {
   while ( ! terminated ) {
      // get data from queue, if queue is empty; wait.
      auto_ptr< PreCacheRouteData > routeData( m_routeData.dequeue( MAX_UINT32 ) );
      if ( routeData.get() == NULL ) {
         continue;
      }
      // create tiles for route
      addRoute( *routeData );
   }
   getGroup()->preCacheDone( this );
}

void PreCacheTileMapHandler::addRoute( const PreCacheRouteData& data ) {
   const ServerTileMapFormatDesc* desc = 
      m_group->
      getTileMapFormatDesc( STMFDParams( data.m_language, false ), this );

   // calculate tilemap params strings
   set<MC2SimpleString> params;
   for ( uint32 i = 0 ; i < data.m_routes.size() ; ++i ) {
      
      PreCacheRouteData::SubData* subData = data.m_routes[ i ];

      uint32 scale = ::getScaleFromSpeedKph( subData->m_speedLimit );
      desc->getAllParams( params, 
                          subData->m_coords.begin(),
                          subData->m_coords.end(),
                          data.m_extraPixels, 
                          data.m_layerIDs,
                          true, // Use gzip
                          data.m_language,
                          scale, scale,
                          &data.m_id );

   }

   params.insert( "DYYY" );
   params.insert( "dYYY" ); // night mode
   // Create a query that doesn't care about the data received.
   NullTileMapQuery query( params, Properties::
                           getUint32Property( "PRECACHE_ROUTE_TIMELIMIT" ) );

   getTileMaps( query );
}
