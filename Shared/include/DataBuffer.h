/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NEWDATABUFFER_H
#define NEWDATABUFFER_H

#include "config.h"
#include "AlignUtility.h"
#include "SharedBuffer.h"
#include "sockets.h"

/**
  *   An encapulation of a data buffer. Provides methods to read
  *   and write single bytes, short integers (16bits) and integers 
  *   (32bits). Uses methods to be independent of big or little 
  *   endian architecture.
  *
  */
class DataBuffer : public SharedBuffer {
public:
      /**
        *   Create an empty DataBuffer, only useful together with
        *   restoreFromStream (the >> operator) or memMapFile.
        */
      DataBuffer();

      /**
       *   Create a DataBuffer with specified size.
       *   @param bufSize    The size of the buffer in bytes.
       *   @param quiet      If set, does not report warnings
       *                     about allocating 0 bytes.
       */
      DataBuffer(uint32 bufSize, bool quiet = false);
           
      /**
       *    Create a DataBuffer with specified size and name.
       *    @param bufSize    The size of the buffer in bytes.
       *    @param name       Name that can be printed when writing
       *                      outside the buffer.
       *                      (For debug, will not be copied).
       *    @param quiet      If set, does not report warnings
       *                      about allocating 0 bytes. 
       */
      DataBuffer(uint32 bufSize, const char* name, bool quiet = false);
      
      /**
        *   Create a DataBuffer from a already allocated memory area.
        *   @param   buffer   Pointer to a 32bit aligned memory area.
        *   @param   bufSize  Buffer size in bytes.
        */
      DataBuffer(byte *buffer, uint32 bufSize);

      /**
       *    Copy constructor.
       *    The name will not be copied since we don't know
       *    where that came from. Even if the other buffer is
       *    mmapped then the new one will not be mmapped.
       *    @param other The data buffer to copy the buffer from.
       *    @param shrinkToOffset True if the new buffer should
       *                          be shrinked to the offset of the other
       *                          one.
       */
      DataBuffer(const DataBuffer& other, bool shrinkToOffset = false );

      /**
       *    Copy constructor.
       *    @param other The data buffer to copy the buffer from.
       *    @param shrinkToOffset True if the new buffer should
       *                          be shrinked to the offset of the other
       *                          one. 
       */
      DataBuffer(const SharedBuffer& other, bool shrinkToOffset = false );

      /**
        *   Delete allocated buffer, if any.
        */
      virtual ~DataBuffer();
  
      /** 
       *    Read the next long from this databuffer, aligns if
       *    necessery.
       *    @return  The next uint32 in the buffer.
       */
      inline uint32 readNextLong();
   /** 
    *    Read the next float32 from this databuffer, aligns if
    *    necessery.
    *    @return  The next float32 in the buffer.
    */
   inline float32 readNextFloat();

      /**
       *    Reads the next long from the databuffer and puts
       *    the value in <code>value</code>.
       *    @param value Where to put the value.
       */
      template<class TYPE> void readNextLong(TYPE& value);

      /**
       *    Reads the next long from the databuffer and puts
       *    the value in <code>value</code>. Assumes buffer to
       *    be aligned.
       *    @param value Where to put the value.
       */
      template<class TYPE> void readNextLongAligned(TYPE& value);

      /** 
       *    Reads the next long from this prealigned databuffer.
       *    @return  The next uint32 in the buffer.
       */
      inline uint32 readNextLongAligned();
      
      /** 
       *    Read a long from this databuffer.
       *    @param index The position to read a long from.
       *    @return  The long at index.
       */
      inline uint32 readLong(uint32 index);

      /** 
       *    Write a long to the end of the buffer.
       *    @param data Data to write in buffer. 
       */
      inline void writeNextLong(uint32 data);

   /** 
    *    Write a float to the end of the buffer.
    *    @param data Data to write in buffer. 
    */
   inline void writeNextFloat( float32 data );

      /** 
       *    Write a long at any position in the buffer.
       *    @param data    Data to write in buffer.
       *    @param index   Index in buffer. 
       */
      void writeLong(uint32 data, uint32 index);
   
      /** 
       *    Get the next short from the buffer, aligns if necessery.
       *    @return  The next uint16 in the buffer.
       */
      inline uint16 readNextShort();

      /**
       *    Reads the next short from the databuffer and puts
       *    the value in <code>value</code>.
       *    @param value Where to put the value.
       */
      template<class TYPE> void readNextShort(TYPE& value);

      /**
       *    Reads the next long from the databuffer and puts
       *    the value in <code>value</code>.
       *    @param value Where to put the value.
       */
      template<class TYPE> void readNextShortAligned(TYPE& value);

      /** 
       *    Get the next short from the buffer.
       *    Assumes buffer is already aligned.
       *    @return  The next uint16 in the buffer.
       */
      inline uint16 readNextShortAligned();

      /** 
       *    Get a short from the buffer.
       *    @param index   The index to read the short at.
       *    @return  The uint16 at index in the buffer.
       */
      inline uint16 readShort(uint32 index);
   
      /** 
       *    Write a short to the end of the databuffer.
       *    @param data The data to write in buffer.
       */
      inline void writeNextShort(uint16 data);   

      /** 
       *    Write a short at any position in the buffer.
       *    @param data    Data to write in buffer.
       *    @param index   Index in buffer.
       */
      void writeShort(uint16 data, uint32 index);
   
      /** 
       *    Get the next byte in the buffer.
       *    @return  The next byte in the buffer.
       */
      inline byte readNextByte();

      /** 
       *    Inser a byte at the end of the buffer.
       *    @param data Data to write in buffer.
       */
      inline void writeNextByte( uint8 value );   
   
      /** 
       *    Read a byte at any position in the buffer.
       *    @param index Index in buffer.
       *    @return the byte read
       */
      byte readByte(uint32 index);

      /** 
       *    Write a byte at any position in the buffer.
       *    @param data Data to write in buffer.
       *    @param index Index in buffer.
       */
      void writeByte(byte data, uint32 index);

      /** 
       *    Read the next boolean value in the buffer. Stored in
       *    one byte.
       *    @return  The next bool in the buffer.
       */
      inline bool readNextBool();

      /** 
       *    Read a bool at any position in the buffer.
       *    @param index Index in buffer.
       *    @return the bool read
       */
      bool readBool(uint32 index);

      /** 
       *    Insert a boolean at the end of the buffer.
       *    @param data Data to write in buffer.
       */
      void writeNextBool(bool data);   

      /** 
       *    Write a boolean at any position in the buffer.
       *    @param data    Data to write in buffer.
       *    @param index   Index in buffer. 
       */
      void writeBool(bool data, uint32 index);

      /** 
       *    Get the length of the next string in the buffer.
       *    @return  The size of the next string in the buffer.
       */
      uint32 getNextStringSize();
      
      /**
        *   Moves the offset to next alligned position (0-3 bytes ahead)
        *   and set the skipped bytes to zero.
        */
     inline void alignToLongAndClear();

      /**
        *   Aligns the buffer to long.
        */
      inline void alignToLong();

      /**
        *   Aligns the buffer to short.
        */
      inline void alignToShort();

      /**
       *    Sets the debug name for the DataBuffer.
       *    Will not be copied so it should be a pointer
       *    to a static one. Will be printed if something
       *    goes wrong.
       */
      inline void setName(const char* name);
   
      /**
        *   Dumps the dataBuffer on the screen.
        *   @param wholeBuffer If true, dump the whole buffer,
        *          otherwise just up to current offset.
        */
      void dump(bool wholeBuffer = false);

      /**
       *    Memory map a file for access via the DataBuffer. <b> Note that
       *    this is not compatible with restoreFromStream/saveToStream!</b>
       *    @param file Path to the file
       *    @param readWrite if true changes to the DataBuffer will be saved
       *           to the file when the DataBuffer is destroyed
       *    @param exitIfFails If true exit will be called if the mmap fails.
       *                       Should be removed when it is examined where it
       *                       is used.
       */
      bool memMapFile(const char* file,
                      bool readWrite = false,
                      bool exitIfFails = true );

      /**
       *    Check if this data buffer equals another.
       *    @param   otherBuffer The other data buffer to compare with.
       *    @return  True if this data buffer equals the other,
       *             false if not.
       */
      bool equals( DataBuffer& otherBuffer ) const;

      /**
       *    Save this databuffer to an ostream. Check the stream
       *    status or use exceptions for error handling.
       *    @param   outStream The stream to use
       */
      void saveToStream(ostream& outStream);

      /**
       *    Restore this databuffer from an istream. Check the stream
       *    status or use exceptions for error handling.
       *    @param   inStream The stream to use
       *    @param   maxBytes Restore max number of bytes
       */
      void restoreFromStream(istream& inStream, uint32 maxBytes = 0);

      /**
       *    Operator to use with ostreams for saving the DataBuffer
       *    @param   outStream  The stream to use
       *    @param   dBuffer    The DataBuffer to save
       *    @return  Reference to outStream
       */
      friend ostream& operator<< (ostream& outStream, DataBuffer& dBuffer);

      /**
       *    Operator to use with ostreams for saving the DataBuffer
       *    @param   outStream  The stream to use
       *    @param   dBuffer    The DataBuffer to restore to
       *    @return  Reference to inStream
       */
      friend istream& operator>> (istream& inStream, DataBuffer& dBuffer);

   /**
    * @return True if the buffer is used with mmap.
    */
   bool isMMaped() const;


protected:
      /**
       *    fd for mmap
       */
      int m_mmapFD;

      /**
       *    Pointer to the name of the buffer.
       */
      const char* m_name;

      /**
       * Prints outside buffer error messages. Implemented in the cpp-file
       * because it should be possible to place a break point in it in gdb.
       *
       * @param methodName The name of the method causing the outside buffer
       *                   error.
       */
      void outsideBufferErrorPrint(const char* methodName) const;
  
private:

      /// Called by constructors.
      void init(uint32 size, const char* name, bool quiet);

      /// Called by constructors.
      void initNotSelfAlloc(byte* buf, uint32 size, uint32 curOffset,
                            const char* name);

};


// ========================================================================
//                                      Implementation of inlined methods =

inline void 
DataBuffer::alignToLongAndClear() 
{
   uint8* newPos = m_pos;
   AlignUtility::alignLong(newPos);
   while (m_pos < newPos) {
      mc2dbg4 << here << " Clearing one byte..." << endl;
      writeNextByte(0);
   }
}

inline void 
DataBuffer::alignToLong() 
{
   AlignUtility::alignLong(m_pos);
}

inline void 
DataBuffer::alignToShort() 
{
   AlignUtility::alignShort(m_pos); 
}

inline void
DataBuffer::setName(const char* name)
{
   m_name = name;
}

// New implementation

inline uint32 
DataBuffer::readNextLongAligned() 
{
#ifdef _DEBUG
   almostAssert((uint32(m_pos) & 3) == 0);
#endif
#if !defined(_MSC_VER) || defined(_DEBUG)
   if ( m_pos > (m_buf + m_bufSize)) {
      outsideBufferErrorPrint("readNextLongAligned");
   }
#endif

#ifdef _WIN32
   uint32 tempLong = *(reinterpret_cast<uint32*>(m_pos));
   m_pos += 4;
   __asm{
      mov eax, tempLong
      bswap eax // ntohl, not for i386 and below.
      mov tempLong, eax
   }
   return tempLong;
#else
   // Read and convert data
   uint32 tempLong = ntohl( *((uint32 *)m_pos));
   m_pos += 4;
   return tempLong;
#endif
}

inline uint32 
DataBuffer::readNextLong() 
{
   // Align to long
   AlignUtility::alignLong(m_pos);

   return readNextLongAligned();
}


inline float32
DataBuffer::readNextFloat() {
   AlignUtility::alignLong(m_pos);
   uint32 l = readNextLongAligned();
   return *reinterpret_cast<float32*>(&l);
}

template<class TYPE>
inline void
DataBuffer::readNextLong(TYPE& value)
{
   value = static_cast<TYPE>(readNextLong());
}

template<class TYPE>
inline void
DataBuffer::readNextLongAligned(TYPE& value)
{
   value = static_cast<TYPE>(readNextLongAligned());
}


inline uint32 
DataBuffer::readLong(uint32 index)
{
   return ntohl( *((uint32 *) (m_buf+index) ));
}

inline 
void DataBuffer::writeNextFloat(float32 data) { 
   AlignUtility::alignLong( m_pos );

   *( (uint32*)(m_pos) ) = htonl( *reinterpret_cast<uint32*>(&data) );
   m_pos += 4;
}

inline 
void DataBuffer::writeNextLong(uint32 data) 
{
   // Align to long
   AlignUtility::alignLong(m_pos);

#ifdef _DEBUG
   almostAssert(!( m_pos > (m_buf + m_bufSize)));
   almostAssert(data != 0xcdcdcdcd); // might be uninitialized.
#endif
   DEBUG1( if( m_pos > (m_buf + m_bufSize) ) {
      outsideBufferErrorPrint("writeNextLong");
   } );
      
   // Convert and write data
   * ( (uint32*)m_pos ) = htonl(data);
   m_pos += 4;

}

inline uint16 
DataBuffer::readNextShortAligned() 
{
   almostAssert((uintptr_t(m_pos) & 1) == 0);
#ifdef _DEBUG
   almostAssert(!( m_pos > (m_buf + m_bufSize)));
#endif
#ifdef _WIN32
   uint16 tempShort = *(reinterpret_cast<uint16*>(m_pos));
   m_pos += 2;
   __asm{
      mov ax, tempShort
      bswap eax  // ntohl
      shr eax, 16 // the result ended up in the higher part of eax. move it to lower part.
      mov tempShort, ax
   }
   return tempShort;
#else
   DEBUG1( if( m_pos > (m_buf + m_bufSize) ) {
      outsideBufferErrorPrint("readNextShort");
   } );

   
   uint16 tempShort = ntohs(*((uint16 *)m_pos));
   m_pos += 2;
#endif
   return tempShort;
}

inline uint16 
DataBuffer::readNextShort() 
{
   // Align to short
   AlignUtility::alignShort(m_pos);

   return readNextShortAligned();
}


template<class TYPE>
inline void
DataBuffer::readNextShort(TYPE& value)
{
   value = static_cast<TYPE>(readNextShort());
}

template<class TYPE>
inline void
DataBuffer::readNextShortAligned(TYPE& value)
{
   value = static_cast<TYPE>(readNextShortAligned());
}


inline uint16 
DataBuffer::readShort(uint32 index)
{
   return ntohs(*((uint16 *) (m_buf+index) ));
}


inline void 
DataBuffer::writeNextShort(uint16 data) 
{
   // Align to short
   AlignUtility::alignShort(m_pos);

#ifdef _DEBUG
   almostAssert(!( m_pos > (m_buf + m_bufSize)));
   almostAssert(data  != 0xcdcd); // might be uninitialized.
#endif
   DEBUG1( if( m_pos  > (m_buf + m_bufSize) ) {
      outsideBufferErrorPrint("writeNextShort");
   } );

   // Convert and write data
   *((uint16 *)m_pos) = htons(data);
   m_pos += 2;
}   



inline byte 
DataBuffer::readNextByte() 
{
#ifdef _DEBUG
   almostAssert(!( m_pos > (m_buf + m_bufSize)));
#endif
   DEBUG1( if( m_pos > (m_buf + m_bufSize)) {
      outsideBufferErrorPrint("readNextByte");
   } );

   // No alignment necessary
   return *m_pos++;
}


inline bool
DataBuffer::readNextBool()  
{
   return readNextByte() != 0;
}


inline void 
DataBuffer::writeNextByte(uint8 data) 
{
   // No alignment necessary
   
   //Convert and write data
#ifdef _DEBUG
   almostAssert(!( m_pos > (m_buf + m_bufSize)));
#endif
   DEBUG1( if( m_pos  > (m_buf + m_bufSize)) {
      outsideBufferErrorPrint("writeNextByte");
   } );
    
   *m_pos++ = data;
}

// ========================================================================
//                                                 DataBufferChecker      =

/**
 *   Class for checking the size and position of DataBuffers.
 *   Uses MC2_ASSERT and shouldn't do much in release version.
 *   E.g. in a save-function in an object: <br />
 *   <code>int MyObject::save(DataBuffer& buf) const            </code><br />
 *   <code>{                                                    </code><br />
 *   <code>   DataBufferChecker dbc(buf, "MyObject::save");     </code><br />
 *   <code>   dbc.assertRoom(getSizeInDataBuffer()              </code><br />
 *   <code>   buf.writeNextLong(...);                           </code><br />
 *   <code>   ...                                               </code><br />
 *   <code>   // All done, check that getSizeInDataBuffer works </code><br />
 *   <code>   dbc.assertPosition(getSizeInDataBuffer());        </code><br />
 *   <code>   return getSizeInDataBuffer();                     </code><br />
 *   <code>}</code><br />
 */
class DataBufferChecker {
  public:
   /**
    *   Creates a new DataBufferChecker with the supplied databuffer.
    *   @param db     The databuffer to check.
    *   @param dbgStr A string to be printed when something goes wrong,
    *                 e.g. the name of the function that is checked.
    */
   inline DataBufferChecker(const DataBuffer& db, const char* dbgStr = NULL);

   /**
    *   Asserts that there is room for <code>nbrBytes</code> more
    *   bytes in the databuffer.
    *   @param nbrBytes The number of bytes that there has to be room
    *                   for from the current position in the DataBuffer.
    */
   inline void assertRoom(uint32 nbrBytes);

   /**
    *   Asserts that the position has advanced offset bytes
    *   since the creation of the DataBufferChecker.
    *   @param offset The wanted offset.
    */
   inline void assertPosition(uint32 offset);
  private:
#ifndef RELEASE_VERSION   
   /// The databuffer to operate on
   const DataBuffer& m_db;

   /// The start position
   uint32 m_startOffset;

   /// The string to print when failing
   const char* m_dbgStr;
#endif
};

inline
DataBufferChecker::DataBufferChecker(const DataBuffer& db, const char* dbgStr)
#ifndef RELEASE_VERSION
      : m_db(db), m_startOffset(db.getCurrentOffset()), m_dbgStr(dbgStr)
#endif
{
}

inline void
DataBufferChecker::assertRoom(uint32 nbrBytes)
{
#ifndef RELEASE_VERSION
   bool condition =
      m_db.getBufferSize() >= (m_db.getCurrentOffset() + nbrBytes);
   if ( m_dbgStr && ! condition ) {
      mc2dbg << "[DataBufferChecker]: " << m_dbgStr
             << "Not enough room for "
             << nbrBytes << " more bytes " << endl;
   }
   MC2_ASSERT( condition );
#endif
}

inline void
DataBufferChecker::assertPosition(uint32 offset)
{
#ifndef RELEASE_VERSION
   bool condition = offset == (m_db.getCurrentOffset() - m_startOffset);
   if ( m_dbgStr && ! condition ) {
      mc2dbg << "[DataBufferChecker]: Position from start in "
             << m_dbgStr << " is "
             << (m_db.getCurrentOffset() - m_startOffset) << " but should be "
             << offset << endl;
   }
   MC2_ASSERT( condition );
#endif
}

#endif
