/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MATHUTILITY_H
#define MATHUTILITY_H
#include "config.h"
#include <math.h>

/**
  *   Static methods concerning mathematical operations.
  * 
  */
class MathUtility {

   public:

      /**
       *    Generic version that doesn't use any math.
       *    @return The number of bits needed to represent 
       *             the unsigned value.
       */
      inline static int getNbrBitsGeneric( uint32 x );
      
      /**
       *    Generic version that doesn't use any math.
       *    @return The number of bits needed to represent 
       *             the signed value.
       */
      inline static int getNbrBitsSignedGeneric( int x );

      /**
       *    Uses getNbrBitsGeneric.
       *    @return The number of bits needed to represent 
       *             the unsigned value.
       */
      inline static int getNbrBits( uint32 value );

      /**
       *    Uses getNbrBitsGeneric.
       *    @return The number of bits needed to represent 
       *             the signed value.
       */
      inline static int getNbrBitsSigned( int signedValue );
};

// ---- Implementation of inlined methods ----
      
inline int 
MathUtility::getNbrBitsGeneric( uint32 x )
{

   int r = 32;
   
   if (x == 0)
      return 1; // 1 bit is needed also for 0...
   if (!(x & 0xffff0000u)) {
      x <<= 16;
      r -= 16;
   }
   if (!(x & 0xff000000u)) {
      x <<= 8;
      r -= 8;
   }
   if (!(x & 0xf0000000u)) {
      x <<= 4;
      r -= 4;
   }
   if (!(x & 0xc0000000u)) {
      x <<= 2;
      r -= 2;
   }
   if (!(x & 0x80000000u)) {
      x <<= 1;
      r -= 1;
   }
   return r;
}

inline int 
MathUtility::getNbrBitsSignedGeneric( int x )
{
   // Ugly fix. Pretend that one bit is needed for the value 0.
   if ( x == 0 ) {
      return 1;
   }

   if ( x < 0 ) { 
      x = -x;
   }
   return getNbrBitsGeneric( uint32( x ) ) + 1;
}

inline int
MathUtility::getNbrBits( uint32 value )
{
   // Removed the use of log and ceil since that didn't work
   // when compiling with optimizations. The generic version is
   // probably faster anyway.
   return getNbrBitsGeneric( value );
}

inline int
MathUtility::getNbrBitsSigned( int signedValue )
{
   // Removed the use of log and ceil since that didn't work
   // when compiling with optimizations. The generic version is
   // probably faster anyway.
   return getNbrBitsSignedGeneric( signedValue );
}

#endif
