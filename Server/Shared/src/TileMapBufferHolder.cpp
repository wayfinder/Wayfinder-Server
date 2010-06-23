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

#include "TileMapBufferHolder.h"
#include "BitBuffer.h"
#include "TileMap.h"
#include "ServerTileMapFormatDesc.h"
#include "Packet.h"

TileMapBufferHolder::TileMapBufferHolder() : 
   m_desc( NULL ), 
   m_buf( NULL), 
   m_crc( MAX_UINT32), 
   m_empty( true ),
   m_good( true ), 
   m_trustAttributes( false ),
   m_mapDesc( NULL )
{

}
      
TileMapBufferHolder::TileMapBufferHolder( const char* desc,
                                          BitBuffer* buf,
                                          uint32 crc,
                                          bool empty ) :
   m_desc( desc ), 
   m_buf( buf ), 
   m_crc( crc ), 
   m_empty( empty ),
   m_good ( true ),
   m_trustAttributes( true ),
   m_mapDesc( NULL )
{

}

TileMapBufferHolder::TileMapBufferHolder( const char* desc,
                                          BitBuffer* buf,
                                          const ServerTileMapFormatDesc* mapDesc ):
   m_desc( desc ), 
   m_buf( buf ), 
   m_crc( MAX_UINT32 ), 
   m_empty( true ),
   m_good ( true ),
   m_trustAttributes( false ),
   m_mapDesc( mapDesc )
{

}

TileMapBufferHolder::TileMapBufferHolder( const Packet* p, int& pos ):
   m_desc( NULL ), 
   m_buf( NULL ), 
   m_crc( MAX_UINT32 ), 
   m_empty( true ),
   m_good ( true ),
   m_trustAttributes( false ),
   m_mapDesc( NULL )
{
   load( p, pos );
}

TileMapBufferHolder::TileMapBufferHolder( const TileMapBufferHolder& other ):
   m_desc( NULL ), 
   m_buf( NULL ), 
   m_crc( MAX_UINT32 ), 
   m_empty( true ),
   m_good ( true ),
   m_trustAttributes( false ),
   m_mapDesc( NULL ) {

   copy( other );
}

TileMapBufferHolder::~TileMapBufferHolder() 
{
   delete m_buf;
}

void
TileMapBufferHolder::updateAttributesIfNeeded()
{
   MC2_ASSERT( m_buf != NULL );
   if ( ! m_trustAttributes ) {
      // crc and empty attributes are not set.
      // Must have a mapdesc in order to be able to load the tilemap.
      MC2_ASSERT( m_mapDesc != NULL );
      TileMap tmap; 

      BitBuffer tmpBuf( m_buf->getBufferAddress(), 
                        m_buf->getBufferSize() );
      tmap.load( tmpBuf, *m_mapDesc, m_desc );
      m_crc = tmap.getCRC();
      TileMapParams param( m_desc );
      uint32 emptyImps = tmap.getEmptyImportances();
      m_empty = static_cast<bool>(emptyImps & (1 << param.getImportanceNbr()));

      // Attributes are now trusted!
      m_trustAttributes = true;
   }
}

bool
TileMapBufferHolder::getEmpty() const
{
   const_cast<TileMapBufferHolder*> (this)->updateAttributesIfNeeded();
   return m_empty;
}

uint32
TileMapBufferHolder::getCRC() const
{
   const_cast<TileMapBufferHolder*> (this)->updateAttributesIfNeeded();
   return m_crc;
}

void 
TileMapBufferHolder::deleteBuffer()
{
   delete m_buf;
   m_buf = NULL;
}

TileMapBufferHolder& TileMapBufferHolder::operator = ( const TileMapBufferHolder& other ) 
{
   copy( other );
   return *this;
}

void TileMapBufferHolder::copy( const TileMapBufferHolder& other, bool skipDesc ) 
{
   if ( &other == this )
      return;

   // create copy to temp before assign (exception safe)
   BitBuffer* buf = NULL;
   if ( other.m_buf != NULL ) {
      buf = new BitBuffer( *other.m_buf );   
   }
   delete m_buf;
   m_buf = buf;

   if ( !skipDesc ) {
      m_desc = other.m_desc;
   }

   m_empty = other.m_empty;
   m_crc = other.m_crc;
   m_good = other.m_good;
   m_trustAttributes = other.m_trustAttributes;
   m_mapDesc = other.m_mapDesc;

}


void
TileMapBufferHolder::load( const Packet* p, int& pos ) {
   // Length of buffer.
   uint32 bufSize = p->incReadLong( pos );

   // Crc
   m_crc = p->incReadLong( pos );
   // Buffer.
   const byte* bufPtr = p->incReadByteArray( pos, bufSize );
   delete m_buf;
   m_buf = new BitBuffer( const_cast<byte*> (bufPtr), bufSize );
   // Desc.
   m_desc = p->incReadString( pos );
   // Empty.
   m_empty = p->incReadByte( pos );
   // Align.
   AlignUtility::alignLong( pos );

   m_good = true;
   m_trustAttributes = true;
}

void
TileMapBufferHolder::save( Packet* p, int& pos ) const {
   // Length of buffer.
   p->incWriteLong( pos, getBuffer()->getCurrentOffset() );
   // Crc
   p->incWriteLong( pos, getCRC() );
   // Buffer.
   p->incWriteByteArray( pos, 
                         getBuffer()->getBufferAddress(),
                         getBuffer()->getCurrentOffset() );
   // Desc.
   p->incWriteString( pos, getDesc() );
   // Empty.
   p->incWriteByte( pos, getEmpty() );
   // Align.
   AlignUtility::alignLong( pos );
}

uint32
TileMapBufferHolder::getSizeInPacket() const {
   uint32 bytes = 3/*padding*/ + 2*4/*crc, bufSize*/ + 1/*Empty*/;

   bytes += m_buf ? m_buf->getCurrentOffset() : 0;
   bytes += (m_desc ? strlen( m_desc ) : 0) + 1;

   return bytes;
}
