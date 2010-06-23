/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SFDFILE_H
#define SFDFILE_H

#include "config.h"
#include "SFDLoadableHeader.h"
#include "MC2SimpleString.h"
#include "BitBuffer.h"
#include "MC2CRC32.h"
#include <vector>


class MapBuff {
public:
   MapBuff( const MC2SimpleString& id, BitBuffer* buff )
         : m_id( id ), m_buff( buff ), m_crc( 0 ) {
      if ( m_buff ) {
         m_crc = MC2CRC32::crc32( m_buff->getBufferAddress(), 
                                  m_buff->getBufferSize() );
      }
   }

   MapBuff( const MC2SimpleString& id, BitBuffer* buff, uint32 crc )
         : m_id( id ), m_buff( buff ), m_crc( crc ) {}


   MC2SimpleString& id() { return m_id; }
   const MC2SimpleString& id() const { return m_id; }

   BitBuffer* buff() { return m_buff; }
   const BitBuffer* buff() const { return m_buff; }

   uint32 crc() const { return m_crc; }

   bool operator == ( const MapBuff& other ) const {
      return crc() == other.crc();
   }

private:
   MC2SimpleString m_id;
   BitBuffer* m_buff;
   uint32 m_crc;
};


/**
 * Can load and save a sfd file.
 *
 */
class SFDFile {
public:
   /**
    * Constructs an empty SFDFile.
    */
   SFDFile( const MC2SimpleString& name, bool debug );

   /**
    * Destructor, deletes all contained buffers.
    */
   ~SFDFile();

   /**
    * Clears and deletes all in this.
    *
    * @param deleteBuffers If to delete buffers or not, used if you want
    *                      the buffers in this after deleting this.
    */
   void clear( bool deleteBuffers = true );

   /**
    * Transfers all buffers and variables to this and
    * clears all buffers without deleting any buffers in other.
    */
   void assign( SFDFile& other );

   /**
    * Set the name.
    */
   void setName( const MC2SimpleString& name );

   /**
    * Load.
    */
   bool load( BitBuffer& buff );

   /**
    * Save.
    *
    * @param buff The buffer to save to.
    * @param wfdVersion The version, only version 0 supported by clients now.
    */
   bool save( BitBuffer& buff, int wfdVersion );

   /**
    * Merge 
    * Assumes that the final, after more merges, SFDFile covers a square
    * area.
    * Might also assume other less obvious things.
    *
    * @param other The SFDFile to merge with this, buffers might be
    *              moved from other so it might not be useable after this.
    */
   bool merge( SFDFile& other );

private:
   /**
    * The header.
    */
   SFDLoadableHeader m_header;

   typedef vector<MC2SimpleString> stringVect_t;

   /**
    * The header strings.
    */
   stringVect_t m_headerStrings;

   typedef vector< MapBuff > mapBuff_t;
   /**
    * The header maps.
    */
   mapBuff_t m_headerMaps;

   /**
    * The MultiBuffer maps.
    * In detail and then lat, lon, layer and importance order.
    */
   mapBuff_t m_maps;
};


#endif // SFDFILE_H

