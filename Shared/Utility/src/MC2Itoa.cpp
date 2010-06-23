/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2Itoa.h"
#include <string>
#include <cstdlib>
#include <algorithm>
#include <cassert>
 
namespace MC2Itoa {

MC2String const characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz/+";
 
MC2String
toStr( uint32 nbr, int base, uint32 padToSize ) {
   MC2String result;
   if ( nbr == 0 ) {
      result = "0";
   }
   long long lbase = base;
   while ( nbr > 0 ) {
      std::lldiv_t temp = std::div( nbr, lbase );
      result += characters[ temp.rem ];
      nbr = temp.quot;
   }
   if ( padToSize != MAX_UINT32 && result.size() < padToSize ) {
      result.insert( result.end(), padToSize - result.size(), '0' );
   }
   std::reverse( result.begin(), result.end() );

   return result;
}
 
uint32 
fromStr( const MC2String& str, int base ) {
   uint32 result = 0;
   for ( size_t i = 0, endNbr = str.size() ; i < endNbr ; ++i ) {
      result = result * base + characters.find( str[ i ] );
   }
   return result;
}

} // End namespace MC2Itoa

