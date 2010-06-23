/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BITBUFFER_H
#define BITBUFFER_H

#include "SharedBuffer.h"

/**
 *   Buffer that can keep track of bits.
 */
class BitBuffer : public SharedBuffer {
public:
   /**
    *   Creates a new buffer with the supplied size.
    */
   BitBuffer(uint32 size);

   /**
    *   Creates a new buffer borrowing the bytes.
    */
   BitBuffer( uint8* bytes, uint32 size );

   /**
    *    Copy constructor.
    *    @param other The data buffer to copy the buffer from.
    *    @param shrinkToOffset True if the new buffer should
    *                          be shrinked to the offset of the other
    *                          one.
    */
   BitBuffer( const BitBuffer& other, bool shrinkToOffset = false );

   /**
    *    Copy constructor. The bitoffset will be reset.
    *    @param other The data buffer to copy the buffer from.
    *    @param shrinkToOffset True if the new buffer should
    *                          be shrinked to the offset of the other
    *                          one.
    */
   BitBuffer( const SharedBuffer& other, bool shrinkToOffset = false );

   /**
    *   Resets all counters to 0.
    */
   void reset();

   /**
    *   Sets the bit-mask to start value and increases
    *   the position if needed.
    */
   void alignToByte();

   /**
    *   Writes the bits to the stream. 1-32 bits!
    */
   inline void writeNextBits(uint32 value, int nbrBits);

   /**
    *   Reads the number of bits from the stream.
    *   @param nbrBits The number of bits to read. 1-32.
    */
   inline uint32 readNextBits(int nbrBits);

   /**
    *   Reads the number of bits from the stream.
    *   @param val     The value to assign.
    *   @param nbrBits The number of bits to read 1-32.
    */
   template<class TYPE> void readNextBits(TYPE& val, int nbrBits) {
      val = static_cast<TYPE>(readNextBits(nbrBits));
   }
   
   /**
    *   Reads the number of bits from the stream.
    *   @param nbrBits The number of bits to read, 1-32.
    */
   inline int32 readNextSignedBits(int nbrBits);
   
   /**
    *   Reads the number of bits from the buffer.
    */
   template<class TYPE> void readNextSignedBits(TYPE& val, int nbrBits) {
      val = static_cast<TYPE>(readNextSignedBits(nbrBits));
   }
   
   /** 
    *    Get the current bit position in the buffer. This is the same
    *    as the number of bits written.
    *    @return The current bit offset in buffer.
    */
   inline uint32 getCurrentBitOffset() const;

private:

   /**
    *   Marks that bit functions have been used. Not
    *   really, really const.
    */
   inline void bitsHaveBeenUsed() const;
   
   /**
    *   Writes one bit.
    */
   inline void writeNextBit(int value);

   /**
    *   Reads one bit.
    */
   inline int readNextBit();
   
   /**
    *   Counter for the bit position in a byte.
    *   32 bits for thumb to be happy.
    */
   uint32 m_bitMask;
   
};

inline void
BitBuffer::bitsHaveBeenUsed() const
{
   DEBUG1(const_cast<BitBuffer*>(this)->m_bytesOK = false);
}

inline int
BitBuffer::readNextBit()
{
   bool retVal = (*m_pos & m_bitMask) != 0;

   m_bitMask >>= 1;

   if ( m_bitMask == 0x00 ) {
      m_bitMask = 0x80;
      m_pos++;
   }

   return retVal;
}

inline uint32
BitBuffer::readNextBits(int nbrBits)
{
   MC2_ASSERT(nbrBits > 0);
   bitsHaveBeenUsed();
   // This could probably be optimized.
   uint32 value = 0;
   while ( nbrBits-- ) {
      value <<= 1;
      value |= readNextBit();
   }
   return value;
}

inline int32
BitBuffer::readNextSignedBits(int nbrBits)
{
   bitsHaveBeenUsed();
   // This could probably be optimized.
   // Add the sign-extension if needed.
   uint32 value = readNextBit();
   if ( value ) {
      value = 0xffffffff;
   }
   // Skip one bit and read the others as usual.
   while ( --nbrBits ) {
      value <<= 1;
      value |= readNextBit();
   }
   return int32(value);
}

inline void
BitBuffer::writeNextBit(int value)
{
   if ( value ) {
      *m_pos |= m_bitMask;      
   } else {
      *m_pos &= (0xff ^ m_bitMask);
   }
   m_bitMask >>= 1;

   if ( m_bitMask == 0x00 ) {
      m_bitMask = 0x80;
      m_pos++;
   }

   
}

inline void
BitBuffer::writeNextBits(uint32 value, int nbrBits)
{
   bitsHaveBeenUsed();
   value <<= (32-nbrBits);
   while ( nbrBits-- ) {
      writeNextBit( (value & 0x80000000) != 0 );
      value <<= 1;
   }
}

inline uint32 
BitBuffer::getCurrentBitOffset() const
{
   uint32 nbrBits = (m_pos - m_buf) << 3; // *8

   switch ( m_bitMask ) {
      case ( 0x80 ) :
         return nbrBits;
      case ( 0x40 ) :
         return nbrBits + 1;
      case ( 0x20 ) :
         return nbrBits + 2;
      case ( 0x10 ) :
         return nbrBits + 3;
      case ( 0x08 ) :
         return nbrBits + 4;
      case ( 0x04 ) :
         return nbrBits + 5;
      case ( 0x02 ) :
         return nbrBits + 6;
      case ( 0x01 ) :
         return nbrBits + 7;
      default:
         // This should not happen.
         MC2_ASSERT( false );
         return nbrBits;
   }
}

#endif
