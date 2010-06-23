/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAP_CLIENTPRECACHE_QUERTY_H
#define TILEMAP_CLIENTPRECACHE_QUERTY_H

#include "TileMapQuery.h"

class SharedBuffer;
class MC2SimpleString;
class DBufRequester;
class ServerTileMapFormatDesc;

#include <vector>

/**
 *   Class to use when precaching tilemaps for client use.
 */
class TileMapClientPrecacheQuery : public TileMapQuery {
public:
   /**
    *   Creates new TileMapClientPrecacheQuery.
    */
   TileMapClientPrecacheQuery( const paramSet_t& params,
                               DBufRequester& cache,
                               const ServerTileMapFormatDesc* mapDesc );
   
   /**
    *   Creates new TileMapClientPrecacheQuery.
    */
   TileMapClientPrecacheQuery( const SharedBuffer& params,
                               DBufRequester& cache,
                               const ServerTileMapFormatDesc* mapDesc );

   /**
    *   Deletes internal data.
    */
   virtual ~TileMapClientPrecacheQuery();
   
protected:
   /**
    *   Adds the buffers to the cache.
    */
   int internalAddBuffers( const bufVect_t& bufs );
   
   /// Returns true if the buffer should be added to the client cache.
   virtual bool shouldBeAdded( const bufVect_t::value_type& buf ) const;

private:
   /**
    *   Cache to add stuff to.
    */
   DBufRequester& m_cache;
  
protected:
   ///  The tilemapformatdesc.
   const ServerTileMapFormatDesc* m_mapDesc;
};

#endif
