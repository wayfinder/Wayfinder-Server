/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GUNZIP_UTIL
#define GUNZIP_UTIL

/**
 *   Utility class for Gunzip-function.
 *   Should be portable and multiplatform so don't
 *   do anything without thinking about that.
 *   <br />
 *   FIXME: CRC-check.
 */
class GunzipUtil {
public:

   /**
    *   Returns the gunzipped length of the compressed inBuffer.
    *   @param inBuffer Gzipped inBuffer.
    *   @param inlen    The length of the inBuffer.
    */
   static int origLength(const unsigned char* inBuffer, int inlen);
   
   /**
    *   Tries to uncompress the inbuffer into the outbuffer.
    *   If something goes wrong a negative number is returned
    *   otherwise the length of the uncompressed data.
    *   @param outBuffer The uncompressed data is put here.
    *   @param outlen    The available space in the outbuffer.
    *   @param inbuffer  Buffer to read the compressed data from.
    *   @param inlen     Length of compressed data.
    *   @return Number of bytes consumed.
    */
   static int gunzip(unsigned char* outBuffer, int outlen,
                     const unsigned char* inBuffer, int inlen );
   
   /**
    *   Returns true if the Gunzip-util is properly implemented
    *   on this platform.
    */
   static bool implemented();

   /**
    * Checks if the two first bytes are the magic gzip ones.
    *
    * @param inBuffer Buffer to check if the two first bytes are the
    *                 magic gzip ones. Must be two bytes long!
    * @return True if the two first bytes are the magic gzip ones, false
    *         if not.
    */
   static bool isGzip( const unsigned char* inBuffer );

   /**
    * Checks if the header is ok.
    */
   static int check_header( const unsigned char* header, int len );

private:
#ifdef HAVE_ZLIB
   /**
    *   Returns the gzip uint32 pointed at by the pointer.
    */
   static unsigned int getLong(const unsigned char* bytePtr);
#endif
};

#endif
