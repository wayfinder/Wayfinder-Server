/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GzipUtil.h"

#include "zlib.h"

int
GzipUtil::gzip(unsigned char* p_outBuffer, int p_outlen,
               const unsigned char* p_inBuffer, int p_inlen,
               int compressionLevel)
{
   // Using zlib types.
   uLong outLen   = p_outlen;
   Byte* outData  = p_outBuffer;
   
   // Zlib starts
   int err;
   int pos = 0;
   bool ok = true;
   int level = Z_DEFAULT_COMPRESSION; //Z_BEST_SPEED
   // Set level according to in-value if it is within bounds.
   if ( compressionLevel >= Z_NO_COMPRESSION &&
        compressionLevel <= Z_BEST_COMPRESSION ) {
      level = compressionLevel;
   }
   int strategy = Z_DEFAULT_STRATEGY;
   z_stream s;
   uLong crc = crc32( 0L, Z_NULL, 0);
   static const int gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */
#define OS_CODE  0x03  /* Default Unix */
#define MEM_LEVEL 8
   
   s.next_in = s.next_out = Z_NULL;
   s.avail_in = s.avail_out = 0;
   
   s.msg = 0;
   
   s.zalloc = (alloc_func)0;
   s.zfree = (free_func)0;
   s.opaque = (voidpf)0;
   
   
   // windowBits is passed < 0 to suppress zlib header
   // WARNING: Undocumented feature in zlib!!
   err = deflateInit2( &s, level, Z_DEFLATED, -MAX_WBITS, 
                       MEM_LEVEL, strategy );
   
   if ( err != Z_OK ) {
      ok = false;
   }
   
   // Set indata
   s.next_in  = (Byte*)p_inBuffer;
   s.avail_in = p_inlen;

   uLong outPos = 0L;
   if ( outLen >= 10 ) {
      // Make gzip header
      outData[ 0 ] = gz_magic[0];
      outData[ 1 ] = gz_magic[1];
      outData[ 2 ] = Z_DEFLATED;
      outData[ 3 ] = 0; /*flags*/
      outData[ 4 ] = 0; /*time*/
      outData[ 5 ] = 0;
      outData[ 6 ] = 0;
      outData[ 7 ] = 0;
      outData[ 8 ] = 0; /*xflags*/
      outData[ 9 ] = OS_CODE;
      outPos = 10L;
   } else {
      ok = false;
   }
   
   // Set outbuff
   s.next_out  = outData + outPos;
   s.avail_out = outLen - outPos;
   
   // Compress
   err = deflate( &(s), Z_FINISH ); // Finnish as in flush to outbuff
   if ( err != Z_OK && err != Z_STREAM_END ) {
      ok = false;
   }
   
   // Update crc
   crc = crc32(crc, p_inBuffer, p_inlen);
   
   // Append gzip end (crc+uncompressed length)
   if ( s.avail_out >= 8 ) {
      *s.next_out++ = crc;
      *s.next_out++ = crc>>8;
      *s.next_out++ = crc>>16;
      *s.next_out++ = crc>>24;
      s.total_out += 4;
      int tot = s.total_in;
      *s.next_out++ = tot;
      *s.next_out++ = tot>>8;
      *s.next_out++ = tot>>16;
      *s.next_out++ = tot>>24;
      s.total_out += 4;
   } else {
      ok = false;
      s.next_out = outData + outLen;
   }
   
   pos = s.total_in;
   outPos = (s.next_out - outData);
   
   // Clean zlib stream
   err = deflateEnd ( &s );
   if ( err != Z_OK ) {
   }
   
   if ( ok ) {
      return outPos;
   } else {
      return -1;
   }
}



