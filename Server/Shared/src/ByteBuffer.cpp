/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DataBuffer.h"

#include "ByteBuffer.h"

// ByteBufElement --------------------------------------------

ByteBufElement::ByteBufElement(byte* data,
                               int len,
                               int offset )
{
   m_data        = data;
   m_len         = len;
   m_readPos     = offset;
}

ByteBufElement::~ByteBufElement()
{
   delete [] m_data;
   m_data = (byte*)(0xdeadbeef); // For debug
}

int
ByteBufElement::getNbrBytesLeft() const
{
   return (m_len - m_readPos);
}

int
ByteBufElement::peek(byte* bytePtr, int len)
{
   int nbrBytesToCopy = MIN(getNbrBytesLeft(), len);
   if ( nbrBytesToCopy > 0 ) {
      memcpy(bytePtr, &m_data[m_readPos], nbrBytesToCopy);
      return nbrBytesToCopy;
   } else {
      return 0;
   }
}

int
ByteBufElement::ack(int len)
{
   m_readPos += len;
   return len;
}

// ByteBuffer -----------------------------------------------

ByteBuffer::ByteBuffer() : m_totalSize(0)
{
}

ByteBuffer::ByteBuffer(const ByteBuffer& other)
{
   if ( other.getNbrBytes() == 0 ) {
      m_totalSize = 0;
   } else {    
      // Create one buffer instead of many
      byte* newBuffer = new byte[other.getNbrBytes()];
      other.peek(newBuffer, other.getNbrBytes());
      add(newBuffer, other.getNbrBytes());
   }
}

ByteBuffer::~ByteBuffer()
{
   // Delete the contents of the list.
   for( ByteBufList::iterator it = m_bufferList.begin();
        it != m_bufferList.end();
        ++it ) {
      // Delete the buffer element
      delete *it;      
   }
   // Okidoki.
   m_bufferList.clear();
}

DataBuffer*
ByteBuffer::copyToDataBuffer() const
{
   DataBuffer* retBuf = new DataBuffer(getNbrBytes());
   peek(retBuf->getBufferAddress(), getNbrBytes());
   retBuf->readPastBytes(getNbrBytes());
   return retBuf;
}

int
ByteBuffer::getNbrBytes() const
{
   return m_totalSize;
}

int
ByteBuffer::add(byte buffer[], int len)
{
   if ( len > 0 ) {
      // Only put it in if there is data.
      ByteBufElement* el = new ByteBufElement(buffer, len);
      m_bufferList.push_back(el);
      // Update the total number of bytes in the buffer.
      m_totalSize += len;
   } else {
      delete [] buffer;
   }
   return len;
}

ssize_t
ByteBuffer::write( const byte* data, size_t len )
{
   if ( len > 0 ) {
      // Only write if there is data.
      byte* dataCopy = new byte[len];
      memcpy(dataCopy, data, len);
      add(dataCopy, len); // Will update totalsize too.
   }
   return len;
}


ssize_t
ByteBuffer::write( const byte* data, size_t len, uint32 timeout ) {
   return write( data, len );
}


int
ByteBuffer::peek(byte* data, int len, ByteBufList::const_iterator it) const
{
   int retVal = 0;
   // No more data to read.
   if ( it == m_bufferList.end() )
      return retVal;

   while ( it != m_bufferList.end() ) {
      // We have data to read.
      ByteBufElement* curElem = *it;
      
      // Peek from the element
      int res = curElem->peek(data, len);
      if ( res > 0 ) {
         // Zero and below are invalid.
         // Decrease the remaining length and advance the data pointer.
         len -= res;
         data += res;
         retVal += res;
      }
      
      if ( len == 0 ) {
         return retVal;
      }
      ++it;
   }
   return retVal;
}

int
ByteBuffer::peek(byte* data, int len) const
{
   //mc2dbg << "ByteBuffer::peek(" << uint32(data) << ", " << len << ")\n";
   return peek(data, len, m_bufferList.begin() );
}

int
ByteBuffer::ack(int len)
{
   //mc2dbg << "ByteBuffer::ack(" << len << ")\n";
   int origLen = len;

   int origSize = m_bufferList.size(); // For debug
   
   for( ByteBufList::iterator it = m_bufferList.begin();
        it != m_bufferList.end();
        ++it) {
      ByteBufElement* curElem = *it;
      int nbrLeft = curElem->getNbrBytesLeft();
      int nbrAcked = curElem->ack(MIN(nbrLeft, len));
      // Decrease the number of bytes to ack.
      len -= nbrAcked;
      if ( len == 0 )
         break; // We're done.
   }
   
   // Remove the unused ones
   while ( !m_bufferList.empty() ) {
      ByteBufElement* front = m_bufferList.front();      
      if ( front->getNbrBytesLeft() == 0 ) {
         // Delete the buffer
         m_bufferList.pop_front();
         delete front;
      } else {
         // Only check until non-empty found.
         break;
      }
   }

   mc2dbg8 << "[BB]: Removed " << (origSize - m_bufferList.size())
           << " elements " << endl;

   // Update the total size of the buffer
   m_totalSize -= (origLen - len);
   return origLen - len;
}

int
ByteBuffer::read(byte* data, int len)
{
   // Read withot advancing
   int res = peek(data, len);
   // Advance
   if ( res > 0 )
      ack(res);
   return res;
}
