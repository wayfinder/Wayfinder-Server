/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ProjectionSettings.h"

#include "DrawingProjection.h"

#include <boost/lexical_cast.hpp>

namespace ProjectionSettings {

uint32 getPixelSize( const stringVector& params ) {
   uint32 pixelSize = CylindricalProjection::NBR_PIXELS;

   if ( params.empty() ) {
      return pixelSize;
   }

   try {
      uint32 newPixelSize = boost::lexical_cast<uint32>( *params[ 0 ] );
      if ( CylindricalProjection::isValidPixelSize( newPixelSize ) ) {
         pixelSize = newPixelSize;
      }
   } catch ( const boost::bad_lexical_cast& e ) {
      // ignore it and use default
      mc2log << "[ProjectionSettings] Can not convert:"
             << *params[ 0 ] << " to integer." << endl;
   }

   return pixelSize;
}

} // ProjectionSettings
