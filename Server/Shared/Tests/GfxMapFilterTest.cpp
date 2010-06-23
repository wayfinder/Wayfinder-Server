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

#include "GfxMapFilter.h"
#include "Stack.h"
#include "GfxDataFull.h"

void testFilter( const GfxData& gfx,
                 const Stack& stack,
                 const uint32 *indexOrder ) {
   for ( uint32 i = 0, testIndex = 0; i < stack.getStackSize();
         ++testIndex ) {
      uint32 nextIndex = GfxMapFilter::
         getFirstAngleDifferenceIndex( stack, gfx,
                                       0, i, 300 );
      MC2_TEST_CHECK_EXT( i == indexOrder[ testIndex ],
                          testIndex );
      if ( nextIndex != i ) {
         i = nextIndex;
      } else {
         ++i;
      }
   }
}

MC2_UNIT_TEST_FUNCTION( gfxMapFilterTest ) {
   /*
    * This test makes sure we can handle almost straight lines
    * with only three nodes like this:
    *
    *  o--------o--------o
    *
    *
    */
   Stack stack;
   GfxDataFull gfx;
   gfx.addCoordinate( 664619158, 157271810, true );
   gfx.addCoordinate( 664621940, 157284650 );
   gfx.addCoordinate( 664623010, 157288502 );
   stack.push( 0 );
   stack.push( 1 );
   stack.push( 2 );

   testFilter( gfx, stack, (uint32[]){ 0, 2, 100 } );
   stack.reset();
   stack.push( 0 );
   stack.push( 1 );
   testFilter( gfx, stack, (uint32[]){ 0, 1, 100 } );
}

MC2_UNIT_TEST_FUNCTION( gfxMapFilterTest2 ) {

   /*
    * These coordinates are like a small street like this:
    *     o--------------o
    *     |              |
    *     |              |
    *     |              |
    *     |              |
    *     |              |
    *     |              |
    *     |              |
    *     |              |
    *     o              o
    *
    * The test makes sure we don't create a triangle of it.
    */
   struct Coord {
      int lat;
      int lon;
   } coords[] = {
      { 664651258, 157095474 },
      { 664655110, 157088840 },
      { 664656822, 157092050 },
      { 664652970, 157098898 }
   };


   GfxDataFull gfx;
   Stack stack;
   // create a new poly first, with the first coordinate
   gfx.addCoordinate( coords[ 0 ].lat, coords[ 1 ].lon, true );
   stack.push( 0 );
   for ( uint32 i = 1; i < sizeof ( coords ) / sizeof ( coords[ 0 ] ); ++i ) {
      gfx.addCoordinate( coords[ i ].lat, coords[ i ].lon );
      stack.push( i );
   }

   testFilter( gfx, stack, (uint32[]){ 0, 1, 2, 3 } );
}

