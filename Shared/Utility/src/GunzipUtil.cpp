/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GunzipUtil.h"

#include "zlib.h"

#include "config.h"

bool 
GunzipUtil::isGzip( const unsigned char* inBuffer ) {
   static const int gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */
   
   // Check the gzip magic header 
   return (inBuffer[ 0 ] == gz_magic[ 0 ] &&
           inBuffer[ 1 ] == gz_magic[ 1 ]);
}

#ifdef HAVE_ZLIB

bool
GunzipUtil::implemented()
{
   // Return true only if it is implemented.
   return true;
}


unsigned int
GunzipUtil::getLong(const unsigned char* bytePtr)
{
   return
      (bytePtr[0] <<  0) |
      (bytePtr[1] <<  8) |
      (bytePtr[2] << 16) |
      (bytePtr[3] << 24);
}

int
GunzipUtil::origLength(const unsigned char* inBuffer, int inlen)
{
   const unsigned char* lastLong = inBuffer + inlen - 4;
   if ( lastLong < inBuffer ) {
      return -1;
   } else {
      return getLong(lastLong);
   }
}

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

static int
get_byte_func(const unsigned char* header,
              int& headerpos,
              int headerlen,
              bool& ok)
{
   if ( headerpos >= headerlen ) {
      headerpos++;
      if ( headerpos > headerlen ) {
         // Second eof.
         ok = false;
      }
      return EOF;
   } else {
      return header[headerpos++];
   }   
}

#define get_byte() ( get_byte_func(header, headerPos, len, ok) )

int
GunzipUtil::check_header( const unsigned char* header, int len ) {
   bool ok = true;
   
   int method; /* method byte */
   int flags;  /* flags byte */

   if ( len < 2 ) {
      return -2; // Not enough bytes
   }
   
   if ( ! GunzipUtil::isGzip( header ) ) {
      return -1;
   }

   // Skip magic.
   int headerPos = 2;
   
   if ( len < 4 ) {
      return -2; // Not enough bytes
   }

   // Read rest of it.
   method = get_byte();
   flags  = get_byte();
   
   if (method != Z_DEFLATED || (flags & RESERVED) != 0) {
      return -1;
   }

   /* Discard time, xflags and OS code: */
   for (int i = 0; i < 6; ++i) {
      get_byte();
   }
   
   if ((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
      uInt flaglen  =  (uInt)get_byte();
      flaglen += ((uInt)get_byte()) << 8;
      /* len is garbage if EOF but the loop below will quit anyway */
      while (flaglen-- != 0 && get_byte() != EOF) {
      }
   }
   
   if ((flags & ORIG_NAME) != 0) { /* skip the original file name */
      int c;
      while ((c = get_byte()) != 0 && c != EOF) {
      }
   }
   
   if ((flags & COMMENT) != 0) {   /* skip the .gz file comment */
      int c;
      while ((c = get_byte()) != 0 && c != EOF) {
      }
   }
   
   if ( (flags & HEAD_CRC) != 0 ) {  /* skip the header crc */
      get_byte();
      get_byte();
   }

   if ( ok ) {
      return headerPos;
   } else {
      return -2; // Not enough bytes
   }
}


int
GunzipUtil::gunzip(unsigned char* p_outBuffer, int p_outlen,
                   const unsigned char* p_inBuffer, int p_inlen)
{
   // Zlib starts  
   bool ok = true;

   z_stream s;

   // Set inbuffer
   s.next_in =  (Byte*)p_inBuffer;
   s.avail_in = p_inlen;

   // Set outbuffer
   s.next_out  = p_outBuffer;
   s.avail_out = p_outlen;
   
   s.msg = 0;
   
   s.zalloc = (alloc_func)0;
   s.zfree = (free_func)0;
   s.opaque = (voidpf)0;

   int headerLen = 0;
   {
      headerLen = check_header(p_inBuffer, p_inlen);
      if ( headerLen < 0 ) {
         ok = false;
         // We can return now since we haven't done any zlib stuff.
         return -1;
      } else {
         // Adjust the data size and position wr the header.
         s.next_in   += headerLen;
         s.avail_in  -= headerLen;
      }
   }
   
   {
      int err = inflateInit2(&s, -MAX_WBITS);
      /* windowBits is passed < 0 to tell that there is no zlib header.
       * Note that in this case inflate *requires* an extra "dummy" byte
       * after the compressed stream in order to complete decompression and
       * return Z_STREAM_END. Here the gzip CRC32 ensures that 4 bytes are
       * present after the compressed stream.
       */
      if ( err != Z_OK ) {
         ok = false;
      }
   }

   {
      int err = inflate(&s, 1);
      if ( err != Z_OK && err != Z_STREAM_END ) {
         ok = false;
      }
   }

   {
      int err = inflateEnd ( &s );
      if ( err != Z_OK ) {
         ok = false;
      }      
   }

   if ( ok ) {
      // Return the number of bytes consumed.
      // Add 8 for the crc and buffer length at the end.
      int retLength = s.next_in - p_inBuffer + 8;
      if ( retLength == p_inlen ) {
         return retLength;
      } else {
         mc2log << warn << "GunzipUtil::gunzip inlen is not retlen! " 
                << p_inlen << " != " << retLength << endl;
         return -1;
      }
   } else {
      return -1;
   }
}

#else

bool
GunzipUtil::implemented()
{
   return false;
}

int
GunzipUtil::origLength(const unsigned char* inBuffer, int inlen)
{
   return -1;
}

int
GunzipUtil::gunzip(unsigned char* p_outBuffer, int p_outlen,
                   const unsigned char* p_inBuffer, int p_inlen)
{
   return -1;
}

int
GunzipUtil::check_header( const unsigned char* header, int len ) {
   return -1;
}


#endif // HAVE_ZLIB
