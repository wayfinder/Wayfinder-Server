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

#include "GfxCoordinates.h"

//
// Tests some functionality of the GfxCoordinate class
//

/// Tests the base class for coordinates.
void testGenericCoordinate( const GfxCoordinates& coord,
                            bool is16Bits,
                            uint32 nbrCoords ) {
   MC2_TEST_CHECK( coord.usesCoordinates16Bits() == is16Bits );
   MC2_TEST_REQUIRED( coord.getNbrCoordinates() == nbrCoords );

   int nbrPassedCoords = 0;
   MC2Coordinate coordPoint;
   // test and request a length of 0 from the first
   // coordinate ( this was an actual crash before ).
   bool success =
      coord.getPointOnPolygon( 0, nbrPassedCoords,
                               // point on polygon
                               coordPoint.lat, coordPoint.lon,
                               true ); // fromFirst
   // It should fail if nbr coords is less than two, and
   // it should succeed when nbr coords is two or more for
   // zero length.
   MC2_TEST_REQUIRED( (nbrCoords < 2 && ! success) ||
                      (nbrCoords >= 2 && success) );

   // We should only "pass" the first coord, if we were to
   // request 0 meters from first coordinates
   MC2_TEST_CHECK( (nbrCoords > 1 && nbrPassedCoords == 1) ||
                   (nbrCoords < 2 && nbrPassedCoords == 0) );

   coord.getPointOnPolygon( 1, nbrPassedCoords,
                            // point on polygon
                            coordPoint.lat, coordPoint.lon,
                            true ); // fromFirst
   // TODO: more unit tests for this class.
}

MC2_UNIT_TEST_FUNCTION( gfxCoordinatesTest ) {
   {
      GfxCoordinates16 shortCoordinate;
      shortCoordinate.addCoordinate( 10, 10,
                                     30, 30 );
      shortCoordinate.addCoordinate( 20, 20 );

      testGenericCoordinate( shortCoordinate,
                             true, // 16 bit
                             2 ); // 2 coords
   }
   {
      GfxCoordinates16 shortCoordinate;
      shortCoordinate.addCoordinate( 10, 10 );

      testGenericCoordinate( shortCoordinate,
                             true, // 16 bit
                             1 ); // 1 coords
   }

}
