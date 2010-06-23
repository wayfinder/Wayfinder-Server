/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRINGCONVERT_H
#define STRINGCONVERT_H

#include "MC2Exception.h"

/// contains string templated conversion functions
namespace StringConvert {

/// Exception thrown by string conversion functions
class ConvertException : public MC2Exception {
public:

   /// @param what the exception message
   ConvertException( const MC2String& what ):
      MC2Exception( "ConvertException", what ) {}

};

/**
 * Converts a string value to templated type, if it fails it will throw
 * ConvertException.
 *
 * @param stringValue the string that should be converted in to T type and
 *                    assigned to parameter value.
 * @return 
 */
template <typename T>
T convert( const MC2String& stringValue ) throw ( ConvertException );

/**
 * Converts a string value to templated type, if it fails it will throw
 * ConvertException.
 * Note: specialize this one if you dont want to create an extra copy during
 * conversion. See StringConvert.cpp assign<MC2String>
 *
 * @param value the variable to be assigned
 * @param stringValue the string that should be converted in to T type and
 *                    assigned to parameter value.
 */
template <typename T>
void assign( T& value, const MC2String& stringValue ) throw (ConvertException)
{
   value = convert<T>( stringValue );
}


}

#endif
