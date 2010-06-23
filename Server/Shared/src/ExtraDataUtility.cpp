/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include <stdlib.h>
#include "ExtraDataUtility.h"



namespace {

inline MC2String::size_type findFirstValidPos( const MC2String& s,
                                               MC2String::size_type pos )  {
   // while position is valid and we have whitespace characters
   // then increase position
   while( pos < s.size() && 
          ( s[ pos ] == ' ' || s[pos] == '\n'
            || s[ pos ] == '\t' || s[ pos ] == '\r'
            || s[ pos ] == ',' ) ) {
      ++pos;
   }
   return pos;
}

inline MC2String::size_type readNumber( const MC2String& s, 
                                        MC2String::size_type pos,
                                        MC2String& outstr ) {
   // While position is valid and we have number or
   // other numerical stuff...
   while(pos < s.size() &&
         ((s[pos] >= '0' && s[pos] <= '9') ||
          (s[pos] == '-') || (s[pos] == '.') )) {
      outstr += s[pos++];
   }
   return pos;
}

}

bool ExtraDataUtility::readLatLon( const MC2String& s, 
                                   MC2String& outLatitude, 
                                   MC2String& outLongitude ) {
   // Coordinate-definition: "(lat,lon)"
   if ( s.empty() || s[0] != '(' ) {
      return false;
   }

   // Get the latitude and longitude-strings...

   // start at offset 1 to skip the initial '('
   MC2String::size_type pos = ::findFirstValidPos( s, 1 );

   MC2String latitude;
   pos = ::readNumber( s, pos, latitude );

   pos = ::findFirstValidPos( s, pos );

   MC2String longitude;
   pos = ::readNumber( s, pos, longitude );

   pos = ::findFirstValidPos( s, pos );
   if ( s[ pos ] != ')' ) {
      return false;
   }

   // data was read correct so set outdata
   outLatitude = latitude;
   outLongitude = longitude;

   return true;
}

bool
ExtraDataUtility::strtolatlon(const MC2String& s, int32& lat, int32& lon,
                              CoordinateTransformer::format_t coordType)
{
   MC2String latitude, longitude;
   if( readLatLon( s, latitude, longitude ) ) {
      char** endptr = NULL;
      float64 flat = strtod(latitude.c_str(), endptr);
      if (endptr != NULL) {
         mc2log << error << "Error reading longitude" << endl;
         return false;
      }

      float64 flon = strtod(longitude.c_str(), endptr);
      if (endptr != NULL) {
         mc2log << error << "Error reading longitude" << endl;
         return false;
      }

      // We need to convert to MC2 coordinates first
      CoordinateTransformer::transformToMC2(
            coordType, flat, flon, 0, lat, lon);

      return true;
   }

   return false;
}


