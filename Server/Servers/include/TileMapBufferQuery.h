/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAP_BUFFER_QUERTY_H
#define TILEMAP_BUFFER_QUERTY_H

#include "config.h"

#include "TileMapQuery.h"

class SharedBuffer;

#include "MC2SimpleString.h"
#include <vector>
#include <map>

/**
 *   Class to be used when requesting single and multibuffers.
 */
class TileMapBufferQuery : public TileMapQuery {
public:
   
   /// Creates query for one map.
   TileMapBufferQuery( const MC2SimpleString& tileMapSpec );

   /// Creates query for many maps.
   TileMapBufferQuery( const paramVect_t& specs,
                       uint32 maxSize = MAX_UINT32 );

   /// Deletes internal stuff
   ~TileMapBufferQuery();

   /// Returns the first buffer. To be used when requesting one map
   const SharedBuffer* getSingleBuffer( const MC2SimpleString& str ) const;

   /// Returns a buffer with all maps written to it.
   const SharedBuffer* getMultiBuffer() const;

protected:
   
   /// From TileMapQuery
   int internalAddBuffers( const bufVect_t& bufs );
   /// From TileMapQuery - will return true when there is too much in m_buf.
   bool isDone() const { return false; }
   
private:

   /// Initializes some of the data
   void init();
   /// Creates new multibuffer
   void createMultiBuffer();

   /// Looks for single buffer in the multi and creates it if found.
   const SharedBuffer* createSingleBuf( const MC2SimpleString& param );
   
   /// Buffer where the stuff is added ( while building multibuffer ).
   vector<uint8> m_buf;
   /// Buffer holding the multidata
   SharedBuffer* m_multiBuf;
   /// Buffer pointing to one map
   map<MC2SimpleString, SharedBuffer*> m_singleBufs;
   /// Maximum size of the outbuffer
   uint32 m_maxSize;
};

#endif
