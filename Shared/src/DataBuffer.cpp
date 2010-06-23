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

// Hex dump..
#include "Utility.h"

#ifndef _MSC_VER
#include<sys/stat.h>
#include<sys/mman.h>
#endif

#include <stdlib.h>

void
DataBuffer::init(uint32 bufSize, const char* name, bool quiet)
{
   if (bufSize > 0) {
      m_buf = (uint8*)(new uint32[(bufSize+3) / 4]);
   } else {
      if (!quiet) {
         MC2WARNING("DataBuffer::DataBuffer(0) attempted! "
                    "Allocating 1 element.");
      }
      m_buf = (uint8*)(new uint32[1]);
   }
   m_bufSize = bufSize;
   m_pos = m_buf;
   m_selfAlloc = true;
   m_mmapFD = -1;
   m_name = name;
   if ( m_name == NULL ) {
      m_name = "";
   }
}

DataBuffer::DataBuffer(uint32 bufSize, bool quiet)
      : SharedBuffer(NULL,0)
{
   init(bufSize, NULL, quiet);
}

DataBuffer::DataBuffer(uint32 bufSize, const char* name, bool quiet)
      : SharedBuffer(0,0)
{
   init(bufSize, name, quiet);
}

DataBuffer::DataBuffer( const DataBuffer& other, bool shrinkToOffset )
      : SharedBuffer( other, shrinkToOffset )
{
   m_name   = "";
   m_mmapFD = -1;   
}

DataBuffer::DataBuffer( const SharedBuffer& other, bool shrinkToOffset )
      : SharedBuffer( other, shrinkToOffset )
{
   m_name   = "";
   m_mmapFD = -1;   
}

void
DataBuffer::initNotSelfAlloc(byte* buffer,
                             uint32 bufSize,
                             uint32 curOffset,
                             const char* name)
{
   m_buf = buffer;
   m_bufSize = bufSize;
   m_pos = m_buf + curOffset;
   m_selfAlloc = false;
   m_mmapFD = -1;
   // Check alignment of buffer.
   uintptr_t checkBuf = (uintptr_t)m_buf;
   if ( checkBuf & 0x3 ) {
      mc2log << error
             << "   - Buffer supplied to DataBuffer must be 32-bit "
             << "aligned. "
             << endl << "   I'm checking what strlen(NULL) can be..."
             << endl;
      MC2ERROR("Non-aligned buffer supplied to DataBuffer::DataBuffer");
      MC2_ASSERT( false );
   }
   m_name = name;
}

DataBuffer::DataBuffer(byte *buffer, uint32 bufSize) : SharedBuffer(NULL, 0)
{
   initNotSelfAlloc(buffer, bufSize, 0, "");
}

DataBuffer::DataBuffer() : SharedBuffer(NULL, 0)
{
   m_buf = NULL;
   m_pos = NULL;
   m_selfAlloc = true;
   m_mmapFD = -1;
   m_bufSize = 0;
}

bool
DataBuffer::memMapFile(const char* file, bool readWrite, bool exitIfFails )
{
#ifndef _MSC_VER
   int flags = O_RDONLY;
   if (readWrite)
      flags = O_RDWR;
   if ((m_mmapFD = open(file, flags)) != -1) {
      struct stat st;
      if(fstat(m_mmapFD, &st) != 0) {
         mc2log << error << "[DataBuffer] couldn't stat file: " << file
                << endl;
         close(m_mmapFD);
      } else {
         m_bufSize = st.st_size;
         int prot = PROT_READ;
         if (readWrite)
            prot = prot | PROT_WRITE;
         m_buf = (uint8*)mmap(0, m_bufSize, prot, MAP_SHARED, m_mmapFD, 0);
         if ( m_buf == (uint8*)-1 ){
            int errorNbr = errno;
            mc2log << error << "[DataBuffer] mmap failed, errno: " 
                   << errorNbr << ", " << strerror(errorNbr)
                   << endl;
         }
      }
   } else {
      mc2log << error << "[DataBuffer] failed to open file: " << file
             << endl;
   }

   if ((-1 == m_mmapFD) || ((uint8*)-1 == m_buf)) {
      // something went wrong
      if (m_mmapFD != -1) {
         close(m_mmapFD);
         if ( exitIfFails ) {
            mc2log << fatal << "[DataBuffer] failed to mmap file: " << file
                   << endl;         
            exit(1);
         } else {
            mc2dbg << "[DataBuffer]: Failed to mmap file: "
                   << MC2CITE( file ) << endl;
         }
      }

      m_buf = NULL;
      m_bufSize = 0;
      m_pos = NULL;
      m_selfAlloc = false;
      m_mmapFD = -1;
      return false;
   } else {
      m_pos = m_buf;
      m_selfAlloc = true;
      return true;
   }
#else
   mc2log << warn << "DataBuffer::memMapFile() Not supported in win32!"
          << endl;
   return false;
#endif // _MSC_VER
}

DataBuffer::~DataBuffer() 
{
   if(m_selfAlloc) {
      if (m_mmapFD != -1) {
#ifndef _MSC_VER
         munmap((char*)m_buf, m_bufSize);
         close (m_mmapFD);
#else
         mc2log << warn << "DataBuffer::~DataBuffer(): m_mmapFD is > 0, "
                << "this should never happen in win32!" << endl;
#endif
      } else {
         delete [] m_buf;
      }
   }
   m_buf = NULL; // Superclass will get confused otherwise
}

void 
DataBuffer::writeLong(uint32 data, uint32 index) 
{
   // Align to long
   AlignUtility::alignLong(index);
#ifdef _DEBUG
   almostAssert(data  != 0xcdcdcdcd); // might be uninitialized.
   almostAssert(index != 0xcdcdcdcd); // might be uninitialized.
#endif

   if(index > m_bufSize ) {
      MC2ERROR2("DataBuffer::writeLong Writing outside buffer",
                cerr << "   index=" << index << ", bufSize=" 
                     << m_bufSize << endl;);
   }
 
   // Convert and write data
   *( (uint32 *) &m_buf[index] ) = htonl(data);
}

void 
DataBuffer::writeShort(uint16 data, uint32 index) 
{
   // Align to short
   AlignUtility::alignShort(index);

#ifdef _DEBUG
   almostAssert(data  != 0xcdcd); // might be uninitialized.
   almostAssert(index != 0xcdcd); // might be uninitialized.
#endif

   if(index > m_bufSize) {
      MC2ERROR2("DataBuffer::writeShort Writing outside buffer",
                cerr << "   index=" << index << ", bufSize=" 
                     << m_bufSize << endl;);
   }
 
   // Convert and write data
   *((uint16 *)&m_buf[index]) = htons(data);
}   

void 
DataBuffer::writeByte(byte data, uint32 index) 
{
   // No alignment necessary
   
   //Convert and write data
   if( index > m_bufSize) {
      MC2ERROR2("DataBuffer::writeByte Writing outside buffer",
                cerr << "   index=" << index << ", bufSize=" 
                     << m_bufSize << endl;);
   }
   m_buf[index] = data;

}

byte 
DataBuffer::readByte(uint32 index) 
{
   if( index > m_bufSize) {
      MC2ERROR2("DataBuffer::readByte Reading outside buffer",
                cerr << ",  index: " << index << ", bufSize: "
                << m_bufSize << endl;);
   }
   return m_buf[index];
}

/*
bool 
DataBuffer::readNextBool() 
{
   // No alignment necessary

   // Read and convert data
   if( m_pos > (m_buf + m_bufSize) ) {
      MC2ERROR2("DataBuffer::readNextBool Reading outside buffer",
                cerr << "   offset=" << m_pos-m_buf << ", bufSize=" 
                     << m_bufSize << endl;);
   }

   bool tempBool = readNextByte() != 0;
   
   DEBUG8(
      if (tempBool)
         cerr << "readNextBool() returns true" << endl;
      else
         cerr << "readNextBool() returns true" << endl
   );

   return tempBool;
}
*/

void 
DataBuffer::writeNextBool(bool data) 
{

   // No alignment necessary
   
   // Convert and write data
   writeNextByte(data ? 0xff : 0x00);
}

void 
DataBuffer::writeBool(bool data, uint32 index) 
{
   writeByte((data ? 0xff : 0x00), index );
}

bool 
DataBuffer::readBool(uint32 index) 
{
   return readByte(index) != 0;
}

uint32 DataBuffer::getNextStringSize() 
{
   uint32 size = 0;
   char *stringPointer = (char *)(m_buf);
   while( *stringPointer++ !='\0' )
      size++;
   return size;
}

void
DataBuffer::saveToStream(ostream& outStream)
{
   char dBufHeader[8];

   strcpy(dBufHeader, "DBUF");
   * ( (uint32*)&dBufHeader[4] ) = htonl(m_bufSize);

   outStream.write(dBufHeader, sizeof(dBufHeader));
   outStream.write((const char*)m_buf, m_bufSize);
}

void
DataBuffer::restoreFromStream(istream& inStream, uint32 maxBytes)
{
   char dBufHeader[8];
   uint32 length;

   inStream.read(dBufHeader, sizeof(dBufHeader));
   length = ntohl( *( (uint32*)&dBufHeader[4] ));
   if ( (maxBytes != 0) && (length > maxBytes) )
      length = maxBytes;
   dBufHeader[4] = '\0';
   if (strcmp(dBufHeader, "DBUF") == 0) {
      if (m_selfAlloc) {
         delete [] m_buf;
         m_bufSize = length;
         m_buf = (uint8*)(new uint32[(m_bufSize+3) / 4]);
         m_pos = m_buf;
         m_selfAlloc = true;
         inStream.read((char*)m_buf, m_bufSize);
      } else {
         MC2ERROR("DataBuffer::restoreFromStream, attempt to restore to a "
                  "Databuffer that wasn't self-allocated!");
         // stream is corrupted 
         // I'd like to use setstate() here but MSVC seems to be broken
         inStream.clear( inStream.rdstate() | ios::badbit );
      }
   } else {
      inStream.clear( inStream.rdstate() | ios::badbit ); // stream is corrupted
   }
}

ostream&
operator<< (ostream& outStream, DataBuffer& dBuffer)
{
   dBuffer.saveToStream(outStream);
   return outStream;
}

istream&
operator>> (istream& inStream, DataBuffer& dBuffer)
{
   dBuffer.restoreFromStream(inStream);
   return inStream;
}

void 
DataBuffer::dump(bool wholeBuf) 
{
#if 0 // Don't know what this means
   DEBUG8(
      uint16 line;
      uint16 maxLine;
      byte   pos;
      byte   maxPos;

      maxLine = (bufSize+15) / 16;
      for (line = 0; line < maxLine; line++) {
         if (line < 1677216)
            cerr << "0";
         if (line < 16384)
            cerr << "0";
         if (line < 256)
            cerr << "0";
         if (line < 16)
            cerr << "0";
         cerr << hex << line << dec << "0: ";
         maxPos = MIN(offset*4+byteOffset-line*16, 16);
         for (pos = 0; pos < maxPos; pos++) {
            if ((buffer[line*16+pos] & htonl(0xff000000)) < 16)
               cerr << "0";
            cerr << hex << (buffer[line*16+pos] & htonl(0xff000000)) 
                 << dec << " ";
         }
         for (; pos < 16; pos++)
            cerr << "   ";
         for (pos = 0; pos < maxPos; pos++)
            if ((char)buffer[line*16+pos] > 31)
               cerr << (char)buffer[line*16+pos];
            else
               cerr << ".";
         for (; pos < 16; pos++)
            cerr << " ";
         cerr << endl;
      }
   );
#else
   if (wholeBuf)
      HEXDUMP(cerr, m_buf, getBufferSize(), "");
   else
      HEXDUMP(cerr, m_buf, getCurrentOffset(), "");
#endif
}

void
DataBuffer::outsideBufferErrorPrint(const char* methodName) const
{

   mc2log << error << "DataBuffer::" << methodName 
          << " outside buffer."
          << " offset=" << (m_pos-m_buf)
          << ", bufSize=" << m_bufSize
          << ",  name = " << MC2CITE(m_name)
          << endl;
   MC2_ASSERT( false );
}

bool
DataBuffer::equals( DataBuffer& otherBuffer ) const
{
   // First check if the size of the buffers matches
   if ( m_bufSize != otherBuffer.m_bufSize ) {
      return false;
   }

   // Check if the buffers are equal
   if ( memcmp( m_buf, otherBuffer.m_buf, m_bufSize ) == 0 ) {
      return true;
   }

   return false;
}

bool DataBuffer::isMMaped() const {
   return m_mmapFD != -1;
}
