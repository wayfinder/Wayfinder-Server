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

class BitBuffer;
struct impRange_t;
class SFDHeader;

#include "MC2SimpleString.h"
#include "LangTypes.h"
#include "TileMapParams.h"

#include <utility>

class SFDMultiBufferReader {
public:

   /// Type of pair returned.
   typedef std::pair<TileMapParams, BitBuffer*> bufPair_t;
   
   /**
    *   Creates new SFDMultiBufferReader.
    *   Takes over ownership of the buffer.
    */
   SFDMultiBufferReader( BitBuffer* bigbuf, 
                         const TileMapParams& param,
                         const SFDHeader* header );

   /**
    *   Deletes the internal buffer.
    */
   ~SFDMultiBufferReader();

   /**
    *   Returns true if there are more buffers.
    */
   bool hasNext() const;

   /**
    *   Returns the current buffer pair.
    *   The buffer must be  deleted or used
    *   in some way before calling readNext
    *   the next time.
    */
   bufPair_t& getCurrent();
   
   /**
    *   Returns the next  param/buffer pair.
    *   The buffer must be  deleted or used
    *   in some way before calling readNext
    *   the next time.
    *   @return The new current bufpair.
    */
   bufPair_t& readNext( LangTypes::language_t lang );

private:
   
   /**
    *    Read the meta data for next layer, if there are any more layers.
    *    @return If  the reader contains another buffer.
    */
   bool readNextMetaIfNeeded();
   
   /**
    *    Move to the next buffer.
    *    @return If  the reader contains another buffer.
    */
   bool moveToNext();
   
   /// The current buffer to return.
   bufPair_t m_current;
   
   /// The big buffer
   BitBuffer* m_bigBuf;
   
   /// The header.
   const SFDHeader* m_header;

   /// A parameter of one of the tilemap buffers.
   TileMapParams m_prototypeParam;

   /// The importance range.
   const impRange_t* m_impRange;
   
   /// The current importance.
   int m_curImp;

   /// The current layer ID.
   int m_layerID;
   
   /// Nbr of layers.
   uint32 m_nbrLayers;
   
   /// The current layer index.
   uint32 m_curLayer;
  
   /// Bitfield containing the importances that contains data.
   uint16 m_existingImps;
   
   /// If the reader contains another buffer.
   bool m_hasNext;

   /// If the current buffer is a string map or not.
   bool m_strings;

};
