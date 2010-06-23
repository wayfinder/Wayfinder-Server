/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2UnitTestMain.h"

#include "SearchRequestParameters.h"
#include "Packet.h"

#include <algorithm>
#define MC2_UNIT_COMPARE_CONT( containerA, containerB ) \
   MC2_TEST_REQUIRED( containerA.size() == containerB.size() ); \
   MC2_TEST_REQUIRED( std::equal( containerA.begin(), \
                                  containerA.end(), \
                                  containerB.begin() ) )

MC2_UNIT_TEST_FUNCTION( searchRequestParametersTest ) {
   SearchRequestParameters params;
   // insert some dummy values
   SearchRequestParameters::POITypeSet poiTypes;
   poiTypes.insert( ItemTypes::restaurant );
   poiTypes.insert( ItemTypes::church );
   poiTypes.insert( ItemTypes::hotel );

   params.setPOITypes( poiTypes );
   // make sure we actually set the values
   MC2_UNIT_COMPARE_CONT( params.getPOITypes(), poiTypes );

   Packet packet( REQUEST_HEADER_SIZE + params.getSizeInPacket() );
   int pos = REQUEST_HEADER_SIZE;
   params.save( &packet, pos );

   // make sure we have the size calculated correctly.
   MC2_TEST_REQUIRED( pos == REQUEST_HEADER_SIZE + params.getSizeInPacket() );

   // make sure the values was not changed during write to packet
   MC2_UNIT_COMPARE_CONT( params.getPOITypes(), poiTypes );

   SearchRequestParameters readParams;
   pos = REQUEST_HEADER_SIZE;
   readParams.load( &packet, pos );

   MC2_UNIT_COMPARE_CONT( readParams.getPOITypes(), params.getPOITypes() );

}
