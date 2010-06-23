/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SQL_STRINGSTREAM_H
#define SQL_STRINGSTREAM_H

#include "MC2String.h"

#include <sstream>
#include <iosfwd>

namespace SQL {

/**
 * Special stringstream for creating SQL queries.
 * Overrides stringstream for MC2Strings and includes ' around MC2Strings and
 * converts uint8 to uint32 before adding. All other types are streamed as
 * normal.
 * Usage:
 * \code
 * SQL::StringStream str;
 * MC2String astring("hello");
 * uint8 value = 8;
 * str << 1023 << ", " << astring << ", " << value;
 * cout << str;
 * // outputs:
 * // 1023, 'hello', 8
 * \endcode
 *
 */
class StringStream: private std::stringstream {
public:
   StringStream() { }

   /// @param value The value to append to stream
   template <typename T>
   inline StringStream& operator << ( const T& value ) {
      static_cast< std::stringstream& >( *this ) << value;
      return *this;
   }

   /// @see std::stringstream::str()
   MC2String str() const {
      return std::stringstream::str();
   }
};

/// Specializes stream from MC2String
template <>
inline StringStream& StringStream::
operator << <MC2String>( const MC2String& value ) {
   static_cast< std::stringstream& >( *this ) << "'" << value << "'";
   return *this;
}

template <>
inline StringStream& StringStream::
operator << <uint8>( const uint8& value ) {
   static_cast< std::stringstream& >( *this ) << static_cast<uint32>( value );
   return *this;
}

} // SQL

/// Output stream for \c SQL::StringStream.
/// @param ostr The output stream to output the \c sql stream on
/// @param sql The SQL string stream to output in \c ostr
/// @return modified output stream.
std::ostream& operator << ( std::ostream& ostr, const SQL::StringStream& sql );

#endif // SQL_STRINGSTREAM_H


