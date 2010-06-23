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
#include "GfxUtility.h"
#include <cmath>

// A small number for floating point comparisons
#define EPS 1e-6

// Floating point comparison
#define EQUALS(x,y) ( fabs(x-y) < EPS )

// Tests GfxUtility::normalizeAngle()
MC2_UNIT_TEST_FUNCTION( normalizeAngleTest ) {
   float testCases[][2] = { 
      // input and expected output
      { 0.0,       0.0      },
      { 2*M_PI,    0.0      },
      { M_PI,      M_PI     },
      { -M_PI,     M_PI     },
      { 7*M_PI,    M_PI     },
      { -1,        2*M_PI-1 }
   };

   for ( size_t i = 0; i < NBR_ITEMS(testCases); ++i ) {
      double angle    = testCases[i][0];
      double expected = testCases[i][1];
      double actual   = GfxUtility::normalizeAngle( angle );
      MC2_TEST_CHECK( EQUALS( actual, expected ) );
   }
}

// Tests GfxUtility::angleDifference()
MC2_UNIT_TEST_FUNCTION( angleDifferenceTest ) {
   float testCases[][3] = {
      // input, input and expected output
      {        0.0,     0.0,       0.0 },
      {        1.0,     2.0,       1.0 },
      {       -1.0,     1.0,       2.0 },
      {        1.0,    -1.0,       2.0 },
      {   M_PI/2.0,     0.0,  M_PI/2.0 },
      { 3*M_PI/2.0,     0.0,  M_PI/2.0 },
   };

   for ( size_t i = 0; i < NBR_ITEMS(testCases); ++i ) {
      double angle1 = testCases[i][0];
      double angle2 = testCases[i][1];
      double result = testCases[i][2];
      MC2_TEST_CHECK( 
         EQUALS( GfxUtility::angleDifference( angle1, angle2 ),
                 result ) );
   }
}
