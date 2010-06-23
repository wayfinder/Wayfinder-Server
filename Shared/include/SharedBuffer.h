/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef KALLE_SHAREDDATABUFFER_H
#define KALLE_SHAREDDATABUFFER_H

#include "config.h"
#include "NotCopyable.h"

class MC2SimpleString;

/**
 *   Superclass to the BitBuffer and DataBuffer.
 *   Functionality that isn't needed in client
 *   has been removed.
 */
class SharedBuffer: private NotCopyable {
public:
   /**
    *   Create a SharedBuffer with specified size.
    *   @param bufSize    The size of the buffer in bytes.
    */
   SharedBuffer(uint32 bufSize);

   /**
    *   Create a SharedBuffer from a already allocated memory area
    *   which will not be deleted when the SharedBuffer is deleted.
    *   @param   buffer   Pointer to a 32bit aligned memory area.
    *   @param   bufSize  Buffer size in bytes.
    */
   SharedBuffer( uint8* buffer, uint32 bufSize);

   /**
    *   Copies the SharedBuffer to another one.
    *   @param other other buffer.
    *   @param shrinkToOffset True if the other buffer should 
    */
   SharedBuffer( const SharedBuffer& other, bool shrinkToOffset = false );

   /**
    *   Delete allocated buffer, if any.
    */
   virtual ~SharedBuffer();

   /**
    *   Returns true if the offset == size.
    */
   bool bufferFilled() const;
   
   // - Write functions
   
   /**
    *   Writes the 32-bit value. Current position only has to be
    *   byte aligned, BA = byte aligned
    */
   void writeNextBALong( uint32 value );
   
   /**
    *   Writes the 16-bit value. Current position only has to be
    *   byte aligned, BA = byte aligned
    */
   void writeNextBAShort( uint16 value );
   
   /**
    *   Writes the 8-bit value. Current position only has to be
    *   byte aligned, BA = byte aligned
    */
   inline void writeNextBAByte( uint8 value );
   
   /** 
    *    Insert a byte at the end of the buffer. Current has to be
    *    byte aligned.
    *    @param data Data to write in buffer.
    */
   inline void writeNextByte( uint8 value );   
   
   /** 
    *    Insert a string to the end of the buffer.
    *    @param data    The data to write in buffer.
    */
   void writeNextString(const char* data);

   /** 
    *    Insert a string to the end of the buffer.
    *    @param data    The data to write in buffer.
    */      
   void writeNextString( const MC2SimpleString& data );
   
   /**
    *    Insert a byte array to the end of the buffer.
    *    @warning Do not write other data than raw byte arrays with 
    *    this method.
    *    @param data    the array to insert.
    *    @param dataLen number of bytes to write.
    */
   void writeNextByteArray( const byte* data, int dataLen ); 

   /**
    *   xor each byte in the buffer with the bytes from the string.
    */
   void xorBuffer( const char* key );
      
   /**
    *   xor each byte in the buffer with the bytes from the xorStuff.
    */
   void xorBuffer( const byte* xorStuff, int xorStuffLength );

   /**
    *   xor each byte in the buffer with each byte in the other buffer.
    *   @param xorKeyBuf The buffer to use for xor:ing. Offset is ignored.
    *                    The whole key buffer is used if it is shorter than
    *                    this buffer.
    */
   void xorBuffer( const SharedBuffer& xorKeyBuf );
      
   // - Read functions
   
   /**
    *    Read the next byte array from the buffer.
    *    @warning Do not read other data than raw byte arrays with 
    *             this method.
    *    @param data    where to store the byte array.
    *    @param dataLen number of bytes to read.
    *    @return number of bytes read.
    */
   int readNextByteArray( byte* data, int dataLen );

   /**
    *    "Read" the next byte array from the buffer. Does not
    *     copy anything, just returns a pointer and increases
    *     the internal position.
    *     @warning Do not read other data than raw byte arrays
    *              with this method.
    *     @param dataLen The length of the data.
    *     @return A pointer to the data array inside the buffer.
    */
   const byte* readNextByteArray( int dataLen );
   
   /**
    *   Reads the 32-bit value. Current position has
    *   to be byte aligned, BA = byte aligned
    */
   uint32 readNextBALong( );

   /**
    *   Reads the 32-bit value. Current position has
    *   to be byte aligned, BA = byte aligned
    *   @param val     The value to assign.
    */
   template<class TYPE> void readNextBALong(TYPE& val) {
      val = static_cast<TYPE>(readNextBALong());
   }
   
   /**
    *   Reads the 16-bit value. Current position has
    *   to be byte aligned, BA = byte aligned
    */
   uint16 readNextBAShort( );
   
   /**
    *   Reads the 16-bit value. Current position has
    *   to be byte aligned, BA = byte aligned
    *   @param val     The value to assign.
    */
   template<class TYPE> void readNextBAShort(TYPE& val) {
      val = static_cast<TYPE>(readNextBAShort());
   }
   
   /**
    *   Reads the 8-bit value. Current position has
    *   to be byte aligned, BA = byte aligned.
    */
   uint8 readNextBAByte( );

   /**
    *   Reads the 8-bit value.
    */
   uint8 readNextByte();
   
   /**
    *   Reads the 8-bit value. Current position has
    *   to be byte aligned, BA = byte aligned
    *   @param val     The value to assign.
    */
   template<class TYPE> void readNextBAByte(TYPE& val) {
      val = static_cast<TYPE>(readNextBAByte());
   }
   
   /** 
    *    Get a pointer to the next string in the databuffer.
    *    @return  The next string in the buffer.
    */
   const char* readNextString();

   /**
    *    Reads the next string and puts in into target.
    *    @param target The string to put it in
    */
   void readNextString( MC2SimpleString& target );

   
   // - Address and offset functions
   
   /** 
    *    Get a pointer to the internal buffer. This method could
    *    be dangerous if used in wrong way. The main reason to
    *    use this method is to write the buffer to a file descriptor.
    *    @return  Pointer to the address of then buffer.
    */
   inline byte* getBufferAddress() const;
   
   /** 
    *    Get the size of the buffer. This is the maximum number
    *    of bytes that could be written into this buffer.
    *    @return Size of buffer in bytes.
    */
   inline uint32 getBufferSize() const;
   
   /** 
    *    Get the current position in the buffer. This is the same
    *    as the number of bytes written.
    *    @return The current offset in buffer 
    *            in bytes.
    */
   inline uint32 getCurrentOffset() const;

   /**
    *    Returns a pointer to the buffer at the current offset.
    */
   inline uint8* getCurrentOffsetAddress() const;
   
   /**
    *    Returns the number of bytes from the current position to
    *    the end of the buffer.
    *    @return The amount of room left in the buffer.
    */
   inline uint32 getNbrBytesLeft() const;
   
   /**   
    *    Resets the offset.
    */
   inline void reset();

   /**
    *    Skip some bytes in the data buffer.
    *    @param nbrBytes The number of bytes to skip.
    */
   inline void readPastBytes(int nbrBytes);

   /**
    *    Sets the offset to be at the end of the buffer.
    */
   inline void setOffsetToSize();
   
   /**
    *    Sets the size to the current offset.
    *    @param   reallocBuffer  [Optional] If the actual buffer should
    *                            be realloced.
    */
   void setSizeToOffset( bool reallocBuffer = false);

   /**
    *   "Dangerous" method that set the value of the membervariable
    *   selfAlloc. This is used in the destructor to check if the
    *   buffer should be deleted or not!
    *
    *   @param   newVal   The new value of the selfAlloc membervariable.
    */
   inline void setSelfAlloc(bool newVal);
   
   /**
    *    Fill all of the allocated buffer with zeros.
    */
   void fillWithZeros();

   /**
    * Releases memory used by this buffer, similar to
    * std::auto_ptr::release(). The internal pointer will set to
    * NULL and the size to zero, so fetch the size before using this.
    * Use this instead of setSelfAlloc(bool).
    */
   uint8* release();

protected:
   /**
    *    The buffer itself.
    */
   uint8* m_buf;
   
   /**
    *    The size of m_buf.
    */
   uint32 m_bufSize;
   
   /**
    *    The current position in the buffer.
    */
   uint8* m_pos;
   
   /**
    *    True if the buffer is allocated within the DataBuffer.
    */
   bool m_selfAlloc;

   /**
    *    True if it is ok to write bytes. Used for debug an assert.
    */
   bool m_bytesOK;

   /**
    *    Crashes if the bit functions of BitBuffer have been
    *    before using the byte functions without a previous
    *    call to BitBuffer::alignToByte.
    */
   inline void assertByteAligned() const;

   /**
    *    Crashes if the byte position is outside the buffer.
    */
   inline void assertPositionAfter() const;
   
private:   
};

/// Dumps a databuffer to a stream. Use like mc2dbg << db_dump(buf);
class db_dump {
private:
   static uint32 getLen( const SharedBuffer* buf, int len ) {
      if ( len != -1 ) {
         return len;
      }                          
      if ( buf == NULL ) {
         return 0;
      }      
      return buf->getCurrentOffset();
   }
public:
   db_dump( const SharedBuffer& buf, int len = -1 )
         : m_buf( buf ),
           m_len( getLen(&buf, len) ) {
   }
 
   friend ostream& operator<< (ostream& outStream, const db_dump& dumper);
private:
   const SharedBuffer& m_buf;
   uint32 m_len;
};


inline void
SharedBuffer::assertByteAligned() const
{
   DEBUG1( MC2_ASSERT( m_bytesOK ) );
}

inline void
SharedBuffer::assertPositionAfter() const
{
   MC2_ASSERT( m_pos <= (m_buf + m_bufSize ) );
}

inline byte*
SharedBuffer::getBufferAddress() const 
{
   return m_buf;
}

inline uint32 
SharedBuffer::getCurrentOffset() const
{
   assertByteAligned();
   return m_pos - m_buf;
}

inline uint8*
SharedBuffer::getCurrentOffsetAddress() const
{
   assertByteAligned();
   return m_pos;
}

inline uint32 
SharedBuffer::getBufferSize() const
{
   return m_bufSize;
}

inline uint32
SharedBuffer::getNbrBytesLeft() const
{
   return getBufferSize() - getCurrentOffset();
}

inline bool
SharedBuffer::bufferFilled() const
{
   return getCurrentOffset() == getBufferSize();
}

inline void
SharedBuffer::readPastBytes(int nbrBytes)
{
   assertByteAligned();
   m_pos += nbrBytes;
   assertPositionAfter();
}

inline void
SharedBuffer::reset()
{
   m_pos = m_buf;
}

inline void
SharedBuffer::setOffsetToSize()
{
   reset();
   readPastBytes( getBufferSize() );
}

inline void 
SharedBuffer::setSelfAlloc(bool newVal) 
{
   m_selfAlloc = newVal;
}

inline void
SharedBuffer::writeNextByte( uint8 value)
{
   assertByteAligned();
   *m_pos++ = value;
   assertPositionAfter();
}

inline void
SharedBuffer::writeNextBAByte( uint8 value)
{
   writeNextByte(value);
}

inline uint8
SharedBuffer::readNextByte( )
{
   assertByteAligned();
   return *m_pos++;
   assertPositionAfter();
}

inline uint8
SharedBuffer::readNextBAByte( )
{   
   return readNextByte();
}

#endif


