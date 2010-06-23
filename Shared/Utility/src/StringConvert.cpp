/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "StringConvert.h"

#include <stdlib.h>

// holds basic POD object assignments

namespace StringConvert {

// specialized templates for POD 

template <>
MC2String convert<MC2String>( const MC2String& stringValue ) {
   return stringValue;
}

template <>
int32 convert<int32>( const MC2String& stringValue ) {
   char* tmpPtr = NULL;
   int32 tmp = strtoul( stringValue.c_str(), &tmpPtr, 0 );
   if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
      throw ConvertException( MC2String( "Could not convert string \"") +
                              stringValue + "\" to integer" );
   }
   return tmp;
}

template <>
uint32 convert<uint32>( const MC2String& stringValue ) {
   return static_cast<uint32>( convert<int32>( stringValue ) );
}

template <>
uint16 convert<uint16>( const MC2String& stringValue ) {
   return convert<uint32>( stringValue );
}

template <>
double convert<double>( const MC2String& stringValue ) {
   char* tmpPtr = NULL;
   double tmp = strtod( stringValue.c_str(), &tmpPtr );
   if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
      throw ConvertException( MC2String( "Could not convert string \"") +
                              stringValue + "\" to double" );
   }
   return tmp;
}

template <>
float convert<float>( const MC2String& stringValue ) {
   char* tmpPtr = NULL;
   float tmp = strtof( stringValue.c_str(), &tmpPtr );
   if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
      throw ConvertException( MC2String( "Could not convert string \"") +
                              stringValue + "\" to double" );
   }
   return tmp;
}

template <>
bool convert<bool>( const MC2String& stringValue ) {
   if ( strcasecmp( stringValue.c_str(), "true" ) == 0 ||
        stringValue == "1" ) {
      return true;
   } else if ( strcasecmp( stringValue.c_str(), "false" ) == 0 ||
               stringValue == "0" ) {
      return false;
   }

   throw ConvertException( MC2String( "Could not convert string \"") +
                           stringValue + "\" to boolean." );
}

template <>
void assign<MC2String>( MC2String& dest, 
                        const MC2String& fromString ) 
   throw (ConvertException) {
   dest = fromString;
}


}
