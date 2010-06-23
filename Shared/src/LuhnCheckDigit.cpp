/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "LuhnCheckDigit.h"

namespace LuhnCheckDigit {

int luhnDigit( const MC2String& str ) {
   bool alternate = true; // We start at the second right digit, no check digit
   int sum = 0;

   for ( int i = str.size() - 1 ; i > -1 ; --i ) {
      if ( !isdigit( str[ i ] ) ) {
         mc2log << warn << "LuhnCheckDigit::luhnDigit non number in data "
                << MC2CITE( str ) << " returning -1." << endl;
         return -1;
      }

      int n = str[ i ] - '0';

      if ( alternate ) {
         n *= 2;
         if ( n > 9 ) {
            n -= 9; // 18 = 1 + 8 = 9 alt. 18 - 9 = 9
         }
      }
      alternate = !alternate;

      sum += n;
   }

   sum %= 10;
   if ( sum != 0 ) {
      sum = 10 - sum;
   }
   return sum;
}

} // namespace LuhnCheckDigit
