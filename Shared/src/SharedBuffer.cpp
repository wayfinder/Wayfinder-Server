/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SharedBuffer.h"
#include "MC2SimpleString.h"
// Hex dump
#include "Utility.h"

ostream& operator<< (ostream& outStream, const db_dump& dumper)
{
   if ( &dumper.m_buf == NULL ) {
      return outStream << "nullbuffer";
   }
   HEXDUMP( outStream, dumper.m_buf.getBufferAddress(), dumper.m_len, "" );
   return outStream;
}   

#include <string.h>

SharedBuffer::SharedBuffer(uint32 bufSize)
{
   m_bufSize = bufSize;
   if ( bufSize ) {
      m_buf = (uint8*)(new uint32[(bufSize+3) / 4]);
   } else {
      m_buf = NULL;
   }
   m_pos     = m_buf;
   m_selfAlloc = true;
   m_bytesOK   = true;
}

SharedBuffer::SharedBuffer( uint8* buffer, uint32 size )
{
   m_bufSize = size;
   m_buf     = buffer;
   m_pos     = m_buf;
   m_selfAlloc = false;
   m_bytesOK   = true;
}

SharedBuffer::SharedBuffer( const SharedBuffer& other, bool shrinkToOffset )
{
   m_selfAlloc = true;
   m_bytesOK   = other.m_bytesOK;   
   m_bufSize = shrinkToOffset ?
      other.getCurrentOffset() : other.m_bufSize;
   
   m_buf = (uint8*)(new uint32[(m_bufSize + 3) / 4]);
   
   memcpy( m_buf, other.m_buf, m_bufSize );

   m_pos = m_buf + other.getCurrentOffset();

}

SharedBuffer::~SharedBuffer()
{
   if ( m_selfAlloc ) {
      delete [] m_buf;
   }
}

uint32
SharedBuffer::readNextBALong()
{
   assertByteAligned();
   uint32 retVal = ( uint32(m_pos[0]) << 24 ) |
                   ( uint32(m_pos[1]) << 16 ) |
                   ( uint32(m_pos[2]) << 8 )  |
                   ( uint32(m_pos[3]) << 0 );
   m_pos += 4;
   assertPositionAfter();
   return retVal;
}

uint16
SharedBuffer::readNextBAShort()
{
   assertByteAligned();
   uint16 retVal = ( uint16(m_pos[0]) << 8 ) |
                   ( uint16(m_pos[1]) << 0 );
   m_pos += 2;
   assertPositionAfter();
   return retVal;
}

const char* 
SharedBuffer::readNextString() 
{
   assertByteAligned();
   // Pointer to return later
   const char* tempPtr = (char*)m_pos;
   // Find end of string
   while ( *m_pos++ != '\0' ) {
   }
   assertPositionAfter();
   return tempPtr;
}

void
SharedBuffer::readNextString( MC2SimpleString& target )
{
   target = readNextString();
}

void
SharedBuffer::writeNextBALong( uint32 value )
{
   assertByteAligned();
   m_pos[0] = (value >> 24) & 0xff;
   m_pos[1] = (value >> 16) & 0xff;
   m_pos[2] = (value >>  8) & 0xff;
   m_pos[3] = (value >>  0) & 0xff;
   m_pos += 4;
   assertPositionAfter();
}

void
SharedBuffer::writeNextBAShort( uint16 value )
{
   assertByteAligned();
   m_pos[0] = (value >>  8) & 0xff;
   m_pos[1] = (value >>  0) & 0xff;
   m_pos += 2;
   assertPositionAfter();
}

void 
SharedBuffer::writeNextString(const char *data)
{
   assertByteAligned();
   uint32 pos = 0;
   while (data[pos] != '\0') {
      writeNextByte(data[pos++]);
   }
   writeNextByte(0);
   assertPositionAfter();
}

void
SharedBuffer::writeNextString( const MC2SimpleString& data )
{
   writeNextString( data.c_str() );
}

void 
SharedBuffer::writeNextByteArray( const byte* data, int dataLen )
{
   assertByteAligned();
   memcpy( m_pos, data, dataLen );
   m_pos += dataLen;
   assertPositionAfter();
}

int 
SharedBuffer::readNextByteArray( byte* data, int dataLen )
{
   assertByteAligned();
   
   if( data != NULL ) {
      memcpy( data, m_pos, dataLen );
      m_pos += dataLen;
      assertPositionAfter();
      return dataLen;
   }

   return 0;
}

void
SharedBuffer::xorBuffer( const char* key )
{
   if ( key == NULL || key[0] == '\0' ) {
      return;
   }
   
   const char* curChar = key;
   for( uint32 i = 0; i < m_bufSize; ++i ) {
      m_buf[i] ^= *curChar++;
      // Check if we have reached the end of the string.
      if ( *curChar == 0 ) {
         // Back to the start
         curChar = key;
      }
   }
}

void
SharedBuffer::xorBuffer( const byte* xorStuff, int xorStuffLength )
{
   if ( xorStuffLength == 0 ) {
      return;
   }
   int xorStuffPos = 0;
   for( uint32 i = 0; i < m_bufSize; ++i ) {      
      m_buf[i] ^= xorStuff[xorStuffPos++];
      // Check if we have reached the end of the string.
      if ( xorStuffPos >= xorStuffLength ) {
         xorStuffPos = 0;
      }
   } 
}

void
SharedBuffer::xorBuffer( const SharedBuffer& xorKeyBuf )
{
   xorBuffer( xorKeyBuf.getBufferAddress(), xorKeyBuf.getBufferSize() );
}

const byte*
SharedBuffer::readNextByteArray( int dataLen )
{
   byte* oldPos = m_pos;
   m_pos += dataLen;
   assertPositionAfter();
   return oldPos;
}

void 
SharedBuffer::fillWithZeros()
{
   memset( getBufferAddress(), 0, getBufferSize() );
}

void
SharedBuffer::setSizeToOffset( bool reallocBuffer )
{
   m_bufSize = getCurrentOffset();

   if ( reallocBuffer ) {
      // Realloc the buffer.
      uint8* newBuf = (uint8*)(new uint32[(m_bufSize + 3) / 4]);
      
      memcpy( newBuf, m_buf, m_bufSize );
     
      if ( m_selfAlloc ) {
         delete[] m_buf;
      } else {
         // Let the allocator delete the old buffer, 
         // but the new buffer is allocated "by self" now.
         m_selfAlloc = true;
      }
      
      m_buf = newBuf;
      m_pos = m_buf + m_bufSize;
   }
}

uint8* SharedBuffer::release() {
   uint8* ret = m_buf;
   m_buf = NULL;
   m_bufSize = 0;
   m_pos = NULL;
   m_selfAlloc = false;
   m_bytesOK = false;

   return ret;
}

