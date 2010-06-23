/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ALIGNUTILITY_H
#define ALIGNUTILITY_H

#include "config.h"

namespace AlignUtility {

// Makes an uint32 aligned byte array
#define MAKE_UINT32_ALIGNED_BYTE_BUFFER( byteSize )                     \
   reinterpret_cast<byte *>( new uint32[ ( byteSize + 3) / 4 ] )

   
/** 
 *   Aligns the position to the nearest short position.
 *   @param position If the position is odd, then it will 
 *         be increased by one. 
 */
//@{
inline void alignShort( int& position ) {
   position += position & 0x1;
}

inline void alignShort( uint32& position ) {
   position += position & 0x1;
}

inline void alignShort( byte* &position ) {
   position += uintptr_t(position) & 0x1;
}

inline int alignToShort( int position ) {
   alignShort( position );
   return position;
}
//@}
          
/**
 *   Aligns the position to the nearest longword position.
 *   @param position while(position % 4 != 0) position++
 */
//@{
inline void alignLong( int& position ) {
   position += ( 4 - position ) & 0x3;
}

inline void alignLong( uint32& position ) {
   position += ( 4 - position ) & 0x3 ;
}

inline void alignLong( byte* &position ) {
   position += ( 4 - uintptr_t(position) ) & 0x3;
}

inline int alignToLong( int position ) {
   alignLong( position );
   return position;
}
//@}
   
} // AlignUtility

#endif // ALIGNUTILITY_H
