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

#include "TileMapClientPrecacheQuery.h"

#include "MC2SimpleString.h"
#include "BitBuffer.h"
#include "DBufRequester.h"
#include "TileMapParamTypes.h"
#include "ServerTileMapFormatDesc.h"
#include "TileMapBufferHolder.h"

TileMapClientPrecacheQuery::
TileMapClientPrecacheQuery( const paramSet_t& params,
                            DBufRequester& cache, 
                            const ServerTileMapFormatDesc* mapDesc )
      : TileMapQuery( params ),
        m_cache( cache ),
        m_mapDesc( mapDesc )
{
   for ( paramSet_t::const_iterator it = params.begin();
         it != params.end();
         ++it ) {
      cout << *it << endl;
   }
}

TileMapClientPrecacheQuery::
TileMapClientPrecacheQuery( const SharedBuffer& params,
                            DBufRequester& cache, 
                            const ServerTileMapFormatDesc* mapDesc )
      : TileMapQuery( params ),
        m_cache( cache ),
        m_mapDesc( mapDesc )
{
}

TileMapClientPrecacheQuery::
~TileMapClientPrecacheQuery()
{
}

bool
TileMapClientPrecacheQuery::shouldBeAdded( const bufVect_t::value_type& buf )
   const
{
   bool addToCache = true;
   // Don't add empty importances.
   if ( TileMapParamTypes::isMap( buf.first.c_str() ) ) {
      // Load and get if the map is empty.
      BitBuffer *tmpBuf = new BitBuffer( buf.second->getBufferAddress(),
                                         buf.second->getBufferSize() );
      TileMapBufferHolder tmpHolder( buf.first.c_str(),
                                     tmpBuf,
                                     m_mapDesc );
      if ( tmpHolder.getEmpty() ) {
         // Don't add it to the cache since it's empty.
         addToCache = false;
         mc2dbg8 << "[TMCPQ]: Skipping empty " << buf.first << endl;
      }
   }
   return addToCache;
}

int
TileMapClientPrecacheQuery::internalAddBuffers( const bufVect_t& bufs )
{
   bufVect_t::const_iterator it_end = bufs.end();
   for ( bufVect_t::const_iterator it = bufs.begin();
         it != it_end;
         ++it ) {
      
      MC2_ASSERT( it->second != NULL );      

      
      // Have to copy it.
      if ( shouldBeAdded(*it) ) {
         m_cache.release( it->first, new BitBuffer( *it->second ) );
      }
   }
   printStatus();

   return bufs.size();
}
