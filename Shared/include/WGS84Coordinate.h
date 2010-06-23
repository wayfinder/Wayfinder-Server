/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WGS84COORDINATE_H
#define WGS84COORDINATE_H

#include "config.h"
#include <iosfwd>

// Forward
class MC2Coordinate;

/// WGS84 (degree) representation
struct WGS84Coordinate {
   typedef float64 Type;

   static const Type INVALID_COORD_VALUE = MAX_FLOAT64;

   WGS84Coordinate():lat( INVALID_COORD_VALUE ), lon( INVALID_COORD_VALUE ) { }

   WGS84Coordinate( Type latitude, Type longitude ):
      lat( latitude ), lon( longitude ) {
   }

   /**
    * Create a WGS84Coordinate from an MC2Coordinate.
    */
   explicit WGS84Coordinate( const MC2Coordinate& coord );

   /// @return true if the latitude and longitude has valid values
   bool isValid() const { 
      return lat != INVALID_COORD_VALUE && lon != INVALID_COORD_VALUE; 
   }

   Type lat; ///< latitude
   Type lon; ///< longitude
};

std::ostream& operator << ( std::ostream& ostr, const WGS84Coordinate& coord );

#endif // WGS84COORDINATE_H
