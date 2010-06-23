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

#include "NotCopyable.h"

#include <vector>

class BitBuffer;
class TileMapParams;
class SharedBuffer;

/**
 *    Usage: 1. create, 2. addMaps, 3. getBuffer
 */
class SFDMultiBufferWriter: private NotCopyable {
public:
   /// Type of vector of not yet added buffers
   typedef vector<pair<TileMapParams, BitBuffer*> > toAddVect_t;

   /// Constructor
   SFDMultiBufferWriter( uint32 startOffset );
   
   /**
    *   Deletes internal buffer.
    */
   ~SFDMultiBufferWriter();

   /**
    *   Write the added maps to the buffer and
    *   return current offset.
    *
    *   @return Offset after last byte written.
    */
   uint32 writeAdded( bool addDebug = false );
   
   /**
    *   Adds a param and bitbuffer to the big buffer.
    *   @param params Parameters for the buffer.
    *   @param buf    Buffer with map data. Will be deleted.
    */
   void addMap( const TileMapParams& params,
                BitBuffer* buf );

   /**
    *   Returns a buffer containing the added ones.
    *   Maps added, and writeAdded, after first call to getBuffer will 
    *   not be in the SharedBuffer even if getBuffer is called again.
    */
   const SharedBuffer* getBuffer() const;

   static void writeHeader( std::vector<byte>& out, int nbrLayers );
   static void writeParam( std::vector<byte>& out,
                           const TileMapParams& dataParam,
                           const SharedBuffer* dataBuf,
                           const TileMapParams& stringParam,
                           const SharedBuffer* stringBuf,
                           int& lastLayer,
                           uint32& impBitsOffset,
                           bool addDebug );
private:

   /**
    *   Creates the buffer to return in getBuffer;
    */
   const SharedBuffer* createBuffer();
   
   /// The data is stored here
   vector<uint8> m_bytes;
   /// The start offset
   uint32 m_startOffset;
   /// The number of added buffers
   uint32 m_nbrMaps;
   /// The buffer of data.
   SharedBuffer* m_buffer;
   /// Current nbr offset
   uint32 m_curNbrOffset;

   /// Vector of not yet added buffers
   toAddVect_t m_toAdd;
};

