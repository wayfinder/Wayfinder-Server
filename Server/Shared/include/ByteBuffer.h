/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BYTEBYUFFER_H_4711
#define BYTEBYUFFER_H_4711

#include "config.h"
#include "Writeable.h"

#include <list>

class Object;

class DataBuffer;
class ByteBufElement; // Last in this file

/**
 *   A class similar to the ByteArrayInputStream and
 *   ByteArrayOutputStream in java.
 *   It is possible to write small chunks of data into the
 *   ByteBuffer and then reading them in chunks of any size.
 */
class ByteBuffer : public Writeable, private NotCopyable {
public:

   /**
    *   Constructor. Zeroes the totalsize.
    */
   ByteBuffer();

   /**
    *   Copy constructor. Will create one new buffer in the
    *   list even if <code>other</code> has many buffers in
    *   its list.
    */
   ByteBuffer(const ByteBuffer& other);
   
   /**
    *   Destructor. Deletes the contents of the list.
    */
   virtual ~ByteBuffer();

   /**
    *   Copies the byteBuffer to a DataBuffer.
    *   The position in the DataBuffer will be after the
    *   written bytes and the size will be the same as the
    *   size of the ByteBuffer.
    *   The byteBuffer will not be changed.
    */
   DataBuffer* copyToDataBuffer() const;
   
   /**
    *   Writes <code>len</code> bytes of <code>data</code> at the
    *   end of the buffer.
    *   @param data The data to write.
    *   @param len  The length of the data.
    *   @return <code>len</code> if everything is ok. Negative number
    *           if an error occured.
    */
   ssize_t write( const byte* data, size_t len );

   /**
    *   Writes <code>len</code> bytes of <code>data</code> at the
    *   end of the buffer. Calls write without timeout.
    *
    *   @param data The data to write.
    *   @param len  The length of the data.
    *   @param timeout 
    *   @return <code>len</code> if everything is ok. Negative number
    *           if an error occured.
    */
   ssize_t write( const byte* data, size_t len, 
                  uint32 timeout );

   /**
    *   Puts the supplied buffer at the end of the ByteBuffer. The buffer
    *   belongs to the ByteBuffer after this, i.e. no copy is made and
    *   the supplied buffer should not be used anymore.
    *   @param buffer The buffer to write.
    *   @param len    The number of bytes in the buffer to write.
    *   @return <code>len</code> if everything went ok.
    */
   int add(byte buffer[], int len);

   /** 
    *   Puts up to <code>len</code> bytes of data into the
    *   supplied buffer and updates the read position. The bytes
    *   cannot be read again.
    *   @param data Buffer to put the read data into.
    *   @param len  The wanted number of bytes to read.
    *   @return The number of bytes that were actually read.
    */
   int read(byte* data, int len);

   /**
    *   Reads <code>len</code> bytes of data from the buffer, but
    *   does not update the read position. I.e. two subsequent calls
    *   to peek should yield the same result.
    *   @param data Buffer to put the read data into.
    *   @param len  The number of wanted bytes.
    *   @return The number of bytes that were put into <code>data</code>.
    */
   int peek(byte* data, int len) const;

   /**
    *   Tells the ByteBuffer to advance the 
    *   read position by <code>len</code> bytes. Can be used together
    *   with peek when resending data.
    *   @param len The length to advance the read position with.
    *   @return The number of bytes that the read position was advanced.
    */
   int ack(int len);

   /**
    *   Returns the number of bytes in the ByteBuffer.
    *   @return The total number of bytes in the ByteBuffer.
    */
   int getNbrBytes() const;
   
protected:
   /**
    *   Typedef of the list, for convenience.
    */
   typedef list<ByteBufElement*> ByteBufList;

   /**
    *   Recursive function used by peek to get data from the elements.
    *   @param data The data pointer where read bytes should be put.
    *   @param len  The remaining length to be read.
    *   @param it   The current element to peek into.
    */
   int peek(byte* data, int len, ByteBufList::const_iterator it) const;
      
   /**
    *   The list holding the real buffers.
    */
   ByteBufList m_bufferList;

   /**
    *   The total number of bytes in the ByteBuffer.
    */
   int m_totalSize;
 
};

/**
 *   Class describing one buffer-part in the ByteBuffer.
 */
class ByteBufElement {
public:

   /**
    *   Creates a new ByteBufElement.
    *   @param data The data of the element. Will be deleted when the
    *               element is deleted.
    *   @param len  The length of the data.
    *   @param offset The offset in the data to start at.
    *                 The length includes the offset so
    *                 if the length of the whole buffer is
    *                 20 and the offset is 10, positions 10-19 will
    *                 be used.
    */
   ByteBufElement(byte* data,
                  int len,
                  int offset = 0 );

   /**
    *   Deletes the element and the bytes in it.
    */
   virtual ~ByteBufElement();
   
   /**
    *   Copies bytes from the element into the supplied data area.
    *   Returns the minimum of the
    *   number of available bytes and <code>len</code>.
    *   @param bytePtr Place to put the read bytes,
    *   @param len     The number of wanted bytes.
    *   @return The number of bytes that can be read or len,
    *           whichever is smaller. If the return value is <= 0
    *           the pointer to the data cannot be trusted.
    */
   int peek(byte* data, int len);

   /**
    *   Changes the read position forward <code>len</code>
    *   positions. No checks are made, so be careful.
    *   @param len The number of positions that the read position
    *              should be advanced.
    *   @return The number of positions that the read position was
    *           advanced.
    */
   int ack(int len);

   /**
    *   Returns the number of available bytes in the element.
    */
   int getNbrBytesLeft() const;
   
private:

   /** Pointer to the real buffer of the element */
   byte* m_data;

   /** The size of the buffer. */
   int m_len;

   /** The read offset of the buffer. */
   int m_readPos;
   
};

#endif
