/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ISAB_CRC32_H
#define ISAB_CRC32_H

#include "config.h"

/**
 * This is the CRC-32 polynomial.
 * x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + 
 * x^5 + x^4 + x^2 + x + 1
 * = 1 0000 0100 1100 0001 0001 1101 1011 0111 = 0x04c11db7
 * This implmentation should be conformant with the Posix 1003 
 * specification.
 *
 */
class MC2CRC32 {
   public:
      /**
       * Calculates the CRC for a buffer.
       *
       * @param buf The buffer to calculate CRC for.
       * @param len The length of buf.
       * @param crc The start value for the crc, default 0.
       * @return The CRC for buf.
       */
      static uint32 crc32( const byte* buf, uint32 len, uint32 crc = 0 );


   private:
      /**
       * Privte constructor to avoid use.
       */
      MC2CRC32();
      
      /**
       * 0x04c11db7.
       */
      static const uint32 POLYQUOTIENT;

      /**
       * Constructs the precalculated crc values.
       */
      static uint32* makeCRCMap();

      /**
       * Holds the precalculated byte values.
       */
      static uint32* m_crcmap;
};


#endif // ISAB_CRC32_H

