/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BITUTILITY_H
#define BITUTILITY_H


#include "config.h" 

/**
 * Bit methods.
 */
namespace BitUtility {
   /**
    *    Sets a bit in a byte.
    *    @param inbyte   The byte to change.
    *    @param bitnumber The bit to set. LSB is 0.
    *    @param bit       The value to set the bit to.
    *    @return The changed byte.
    */
   inline byte setBit( byte &inbyte, int bitnumber, bool bit );

   /**
    *    Gets a bit from a byte.
    *    @param inbyte The byte to test the bit in.
    *    @param bitnumber The bit to test.  LSB is 0.
    *    @return True if the bit is set.
    */
   inline bool getBit( byte inbyte, int bitnumber );

   /**
    *    Sets a bit in an uint16.
    *    @param inInt     The uint32 to change.
    *    @param bitnumber The bit to set. LSB is 0.
    *    @param bit       The value to set the bit to.
    *    @return The changed uint32.
    */
   uint16 setBit( uint16 &inInt, int bitnumber, bool bit );

   /**
    *    Gets a bit from an uint16.
    *    @param inInt     The uint32 to test the bit in.
    *    @param bitnumber The bit to test.  LSB is 0.
    *    @return True if the bit is set.
    */
   bool getBit( uint16 inbyte, int bitnumber );

   /**
    *    Sets a bit in an uint32.
    *    @param inInt     The uint32 to change.
    *    @param bitnumber The bit to set. LSB is 0.
    *    @param bit       The value to set the bit to.
    *    @return The changed uint32.
    */
   inline uint32 setBit( uint32 &inInt, int bitnumber, bool bit );

   /**
    *    Gets a bit from an uint32.
    *    @param inInt     The uint32 to test the bit in.
    *    @param bitnumber The bit to test.  LSB is 0.
    *    @return True if the bit is set.
    */
   inline bool getBit( uint32 inbyte, int bitnumber );

   /** 
    *    The number of bits set in x.
    * 
    *    @param x The uint32 to count set bits in.
    *    @return The number of bits set in x. 
    */ 
   uint32 nbrBits( uint32 x );

   /**
    * The implementation of setBits method. Don't use this as it might
    * not give the results you expect for all types.
    */
   template<class B>
   inline B BitUtility_setBit_impl( B& inInt, int bitnumber, bool bit );

   /**
    * The implementation of getBits method. Don't use this as it might
    * not give the results you expect for all types.
    */
   template<class B>
   inline bool BitUtility_getBit_impl( B inInt, int bitnumber );
};


// =======================================================================
//                                     Implementation of inlined methods =

template<class B>
inline B 
BitUtility::BitUtility_setBit_impl( B& inInt, int bitnumber, bool bit ) {
   B bitmask = (B(1) << bitnumber);
   if ( bit ) {
      inInt |= bitmask;
   } else {
      inInt &= ~bitmask;
   }
   return inInt;
}

template<class B>
inline bool
BitUtility::BitUtility_getBit_impl( B inInt, int bitnumber ) {
   B bitmask = (B(1) << bitnumber);
   return ((inInt & bitmask) == bitmask);
}

inline byte
BitUtility::setBit( byte &inbyte, int bitnumber, bool bit ) {
   return BitUtility_setBit_impl( inbyte, bitnumber, bit );
}

inline bool
BitUtility::getBit( byte inbyte, int bitnumber ) {
   return BitUtility_getBit_impl( inbyte, bitnumber );
}

inline uint16
BitUtility::setBit( uint16 &inInt, int bitnumber, bool bit ) {
   return BitUtility_setBit_impl( inInt, bitnumber, bit );
}

inline bool
BitUtility::getBit( uint16 inInt, int bitnumber ) {
   return BitUtility_getBit_impl( inInt, bitnumber );
}

inline uint32 
BitUtility::setBit( uint32 &inInt, int bitnumber, bool bit ) {
   return BitUtility_setBit_impl( inInt, bitnumber, bit );
}

inline bool
BitUtility::getBit( uint32 inInt, int bitnumber ) {
   return BitUtility_getBit_impl( inInt, bitnumber );
}

inline uint32
BitUtility::nbrBits( uint32 x ) { 
   x = (x >> 1 & 0x55555555) + (x & 0x55555555); 
   x = (x >> 2 & 0x33333333) + (x & 0x33333333); 
   x = (x >> 4 & 0x0f0f0f0f) + (x & 0x0f0f0f0f); 
   x = (x >> 8 & 0x00ff00ff) + (x & 0x00ff00ff); 
   return (x >> 16) + (x & 0xffff); 
}

#endif // BITUTILITY_H

