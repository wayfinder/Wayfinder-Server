/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PropertyHelper.h"


#include "MC2String.h"

#include <stdio.h>

namespace PropertyHelper {

// Basic properties like integer and MC2String

template <>
int 
getFromString<int>( const char* str ) throw (PropertyException) {
   int val;
   if ( sscanf( str, "%d", &val ) != 1 ) {
      throw PropertyException( 
         MC2String("Could not convert to integer from value: ") + str + "." );
   }

   return val;
}

template <>
unsigned int 
getFromString<unsigned int>( const char* str ) 
   throw (PropertyException) {

   int val;
   if ( sscanf( str, "%u", &val ) != 1 ) {
      throw PropertyException( 
         MC2String("Could not convert to integer from value: ") + str + "." );
   }

   return val;
}

template <>
MC2String 
getFromString<MC2String>( const char* str) 
   throw (PropertyException) {
   return MC2String( str ? str : "" );
}

template<>
uint64 get( const MC2String& propStr, uint64 defaultValue ) {
   return Properties::getUint64Property( propStr, defaultValue );
}

template<>
uint32 get( const MC2String& propStr, uint32 defaultValue ) {
   return Properties::getUint32Property( propStr, defaultValue );
}

template <>
float64 get( const MC2String& propStr, float64 defaultValue ) {
   try {
      return get< float64 >( propStr.c_str() );
   } catch ( ... ) {
   }

   return defaultValue;
}

template<>
bool get( const MC2String& propStr, bool defaultValue ) {
   return Properties::getBoolProperty( propStr, defaultValue );
}

}
