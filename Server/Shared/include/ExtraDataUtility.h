/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTRADATAUTILITY_H
#define EXTRADATAUTILITY_H

#include "config.h"
#include "CoordinateTransformer.h"
#include "MC2String.h"

class ExtraDataUtility {
   public:
      
     /**
      * Parse a string in the format "(number,number)".
      * @param s Input string to be parsed.
      * @param outLatitude Returned parse latitude on success.
      * @param outLongitude Returns parsed longitude on success.
      * @return true if parsing was successful.
      */
      static bool readLatLon( const MC2String& s, 
                              MC2String& outLatitude, 
                              MC2String& outLongitude );
      /**
       *    Get the numeric value of a coordinate in the location token 
       *    '(' <LAT> ',' <LON> ')'.
       *
       *    @param s         The string token to convert.
       *    @param lat       Latitude value.
       *    @param lon       Longitude value.
       *    @param coordType The type of the coordinate to transform.
       *
       *    @return  True upon success, false otherwise.
       */
      static bool strtolatlon(const MC2String& s, int32& lat, int32& lon,
                              CoordinateTransformer::format_t coordType);


};

#endif

