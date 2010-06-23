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
#include "ScopedArray.h"
#include "ReferenceImp/BitBuffer.h"

#include <bitset>

// macro for printing and asserting the buffers
#define SPECIAL_ASSERT( x, y )  \
   if ( x != y ) { \
      cout << "current offset: " << buf.getCurrentBitOffset() \
           << " bits: " << bits << endl; \
      cout << "improved offset: " << newBuf.getCurrentBitOffset() << endl; \
      cout << "pos: "  <<  (int)( buf.getCurrentOffsetAddress() -       \
                                  buf.getBufferAddress() )                \
           << ", " << (int)( newBuf.getCurrentOffsetAddress() - \
                             newBuf.getBufferAddress() )        \
           << endl;                                                     \
      cout << "old data: " << hex << (int)buf.getCurrentOffsetAddress()[0] \
           << " " << (int)buf.getCurrentOffsetAddress()[1]                   \
           << " " << (int)buf.getCurrentOffsetAddress()[2]              \
           << " " << (int)buf.getCurrentOffsetAddress()[3]              \
           << endl << "new data: " << (int)newBuf.getCurrentOffsetAddress()[0] \
           << " " << (int)newBuf.getCurrentOffsetAddress()[1]            \
           << " " << (int)newBuf.getCurrentOffsetAddress()[2]            \
           << " " << (int)newBuf.getCurrentOffsetAddress()[3]            \
           << endl;                                                     \
      cout << dec;       \
      cout << hex; \
      cout << #x << "(" << x << ") != " << #y << "(" << y << ")" << endl; \
      MC2_ASSERT( x == y ); \
   }

/// Tests write speed between reference implementation and new implementation
/// Use with valgrind.
template <typename Data>
void testWriteSpeed( uint32 dataSize ) {

   ScopedArray<uint8> dataOrg( new uint8[ dataSize ] );
   // clear data
   for ( uint32 i = 0; i < dataSize; ++i ) {
      dataOrg[ i ] = 0;
   }

   Data buf( dataOrg.get(), dataSize );

   uint64 bitsRead = 0;
   const uint32 MAX_BITS = dataSize * 8;

   while ( bitsRead < MAX_BITS ) {
      uint32 bits = rand() % 32 + 1;
      uint32 value = rand() % ( 8 * bits );
      bitsRead += bits*2 + 1;
      
      if ( bitsRead > MAX_BITS ) {
         break;
      }

      buf.writeNextBits( value, bits );
      
      if ( rand() % 100 > 50 ) {
         buf.alignToByte();
      }
   }
}

void testReadNextByte() {
   unsigned char *buff = new unsigned char[2];
   buff[0] = 13;
   buff[1] = 179;

   BitBuffer bb1( buff, 2 );
   ReferenceImp::BitBuffer bb2(buff, 2);

   bb1.readNextBits( 8 );
   bb2.readNextBits( 8 );
   MC2_ASSERT(bb1.readNextByte() == bb2.readNextByte());
   delete [] buff;
}

int main( int argc, char** argv ) {
   if ( argc < 2 ) {
      cerr << "Usage: " << argv[ 0 ] << " <buffer size> " << endl;
      return 1;
   }

   testReadNextByte();

   // Seed the randomizer
   const uint32 seed = time( NULL );
   const uint32 dataSize = atoi( argv[ 1 ] );
   cout << "BitBufferTest: Size: " << dataSize << " Seed: " << seed << endl;
   srand( seed );

   // Test write speed ( with valgrind )
   //   testWriteSpeed<BitBuffer>( atoi( argv[ 1 ] ) );

   // create empty data set
   ScopedArray<uint8> dataOrg( new uint8[ dataSize ] );
   ScopedArray<uint8> dataNew( new uint8[ dataSize ] );

   // clear data
   for ( uint32 i = 0; i < dataSize; ++i ) {
      dataNew[ i ] = 0;
      dataOrg[ i ] = 0;
   }

   ReferenceImp::BitBuffer buf( dataOrg.get(), dataSize );
   BitBuffer newBuf( dataNew.get(), dataSize );

   uint64 bitsRead = 0;
   const uint32 MAX_BITS = dataSize * 8;
   mc2dbg << "BitBufferTest: maximum bits to read: " << MAX_BITS << endl;
   mc2dbg << "BitBufferTest: Write Test." << endl;
   // Test write
   while ( bitsRead < MAX_BITS ) {
      uint32 bits = rand() % 32 + 1;
      uint32 value = rand() % ( 8 * bits );
      bitsRead += bits*2 + 1;
      
      if ( bitsRead > MAX_BITS ) {
         break;
      }

      buf.writeNextBits( value, bits );
      newBuf.writeNextBits( value, bits );

      SPECIAL_ASSERT( buf.getCurrentBitOffset(), newBuf.getCurrentBitOffset() );
      
      if ( rand() % 100 > 50 ) {
         buf.alignToByte();
         newBuf.alignToByte();
      }

   }

   MC2_ASSERT( memcmp( dataOrg.get(), dataNew.get(), dataSize ) == 0 );

   buf.reset();
   newBuf.reset();
   bitsRead = 0;
   int bits = 0;
   SPECIAL_ASSERT( buf.getCurrentBitOffset(), newBuf.getCurrentBitOffset() );

   mc2dbg << "BitBufferTest: Read Test." << endl;
   // Read random number of bits until we read them all.
   while ( buf.getCurrentBitOffset() < MAX_BITS ) {
      uint32 bits = rand() % 32 + 1;

      // too much?
      if ( buf.getCurrentBitOffset() + 2 * bits  > MAX_BITS ) {
         break;
      }

      // read unsigned value
      uint32 orgRead = buf.readNextBits( bits );
      uint32 newRead = newBuf.readNextBits( bits );
      SPECIAL_ASSERT( orgRead, newRead );
      SPECIAL_ASSERT( buf.getCurrentBitOffset(), newBuf.getCurrentBitOffset() );
      bitsRead += bits;

      // read signed value
      int orgSRead = buf.readNextSignedBits( bits );
      int newSRead = newBuf.readNextSignedBits( bits );
      SPECIAL_ASSERT( orgSRead, newSRead );
      SPECIAL_ASSERT( buf.getCurrentBitOffset(), newBuf.getCurrentBitOffset() );

      bitsRead += bits;
      
      // test alignToByte
      if ( rand() % 100 > 50 ) {
         buf.alignToByte();
         newBuf.alignToByte();
      }

   }
   // read the final bits
   while ( buf.getCurrentBitOffset() < MAX_BITS ) {
      BitBuffer::SizeType bits = MAX_BITS - buf.getCurrentBitOffset();
      if ( bits > 32 ) {
         bits = 32;
      }
      uint32 orgRead = buf.readNextBits( bits );
      uint32 newRead = newBuf.readNextBits( bits );
      SPECIAL_ASSERT( orgRead, newRead );
      SPECIAL_ASSERT( buf.getCurrentBitOffset(), newBuf.getCurrentBitOffset() );

  }

   mc2dbg << "final bit offset: " << buf.getCurrentBitOffset() << endl;
  return 0;
}
