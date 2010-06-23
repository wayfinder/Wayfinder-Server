/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BitBuffer.h"

namespace ReferenceImp {

BitBuffer::BitBuffer(uint32 size)
   : SharedBuffer(size)
{
   BitBuffer::reset();
}

BitBuffer::BitBuffer( uint8* bytes, uint32 size )
   : SharedBuffer(bytes, size)
{
   BitBuffer::reset();
}

BitBuffer::BitBuffer( const BitBuffer& other, bool shrinkToOffset )
   : SharedBuffer( other, shrinkToOffset )
{
   m_bitMask = other.m_bitMask;   
}

BitBuffer::BitBuffer( const SharedBuffer& other, bool shrinkToOffset )
   : SharedBuffer( other, shrinkToOffset )
{
   m_bitMask = 0x80;
}

void
BitBuffer::reset()
{
   m_pos     = m_buf;
   m_bitMask = 0x80;
   m_bytesOK = true;
}


void
BitBuffer::alignToByte()
{
   if ( m_bitMask != 0x80 ) {
      // Not done writing bits to this byte.
      // Skip to next one.
      ++m_pos;
   }
   m_bitMask = 0x80;
   m_bytesOK = true;
}

}
