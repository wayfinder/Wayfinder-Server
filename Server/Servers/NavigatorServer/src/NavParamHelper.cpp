/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavParamHelper.h"
#include "NParam.h"
#include <math.h>

namespace NavParamHelper {

bool getCoordAndAngle( const NParam* param, MC2Coordinate& coord,
                       uint16& angle ) {
   if ( param != NULL && param->getLength() >= 8 ) {
      coord = MC2Coordinate( Nav2Coordinate( param->getInt32Array( 0 ), 
                                             param->getInt32Array( 1 ) ) );
      if ( param->getLength() >= 10 ) {
         angle = param->getUint16( 8 );
         if ( angle <= 255 ) {
            angle = uint16( rint( angle * 360.0 / 256.0 ) );
         } else {
            angle = MAX_UINT16;
         }
      }
      return true;
   } else {
      return false;
   }
}

bool getMC2CoordAndAngle( const NParam* param, MC2Coordinate& coord,
                          uint16& angle ) {
   if ( param != NULL && param->getLength() >= 8 ) {
      coord = MC2Coordinate( param->getInt32Array( 0 ), 
                             param->getInt32Array( 1 )  );
      if ( param->getLength() >= 10 ) {
         angle = param->getUint16( 8 );
         if ( angle <= 255 ) {
            angle = uint16( rint( angle * 360.0 / 256.0 ) );
         } else {
            angle = MAX_UINT16;
         }
      }
      return true;
   } else {
      return false;
   }
}

}
