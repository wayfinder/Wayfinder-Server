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

#include "MaxCount.h"

MC2_UNIT_TEST_FUNCTION( maxCountTest ) {
   using TalkerUtility::maxCount;

   const uint32 PAGE_SIZE = 10;
   const uint32 LAST_OFFSET = PAGE_SIZE*4;
   
   uint32 end = PAGE_SIZE + PAGE_SIZE/2;

   // we request more than a page can hold
   // but we get less than a page and the estimated
   // result count is larger.
   int val = maxCount( 0, end,  // start, end
                       1, // reply hits
                       720, // estimated result count
                       LAST_OFFSET, PAGE_SIZE );
   MC2_TEST_CHECK_EXT( val == 1, val );

   // We request more than a page can hold
   // and we should get a full page back with the
   // estimated count as the result.
   val = maxCount( 0, end, // start, end
                   PAGE_SIZE, // reply hits
                   720, // estimated result count
                   LAST_OFFSET, PAGE_SIZE );
   MC2_TEST_CHECK_EXT( val == 720, val );

   // We request 1.5 page and but only got half a page back.
   // This means that we should expect a return value of half a page.
   MC2_TEST_CHECK( maxCount( 0, end, // start, end:
                             PAGE_SIZE/2, // reply hits
                             640, // estimated result count
                             LAST_OFFSET, PAGE_SIZE ) == PAGE_SIZE/2 );

   // We request half of the second page and got half back
   // but the max hits should be in the estimated range
   MC2_TEST_CHECK( maxCount( PAGE_SIZE, end, //start, end
                             PAGE_SIZE/2,
                             640, // est
                             LAST_OFFSET, PAGE_SIZE ) == 640 );

   // We request the last page and we get a full page back
   // and the max value should then be the last offset so we
   // do not request any more after this.
   MC2_TEST_CHECK( maxCount( LAST_OFFSET - PAGE_SIZE, LAST_OFFSET, 
                             PAGE_SIZE,
                             320,
                             LAST_OFFSET, PAGE_SIZE ) == LAST_OFFSET );

   // Now try the same as above, but with the end index inside the last
   // offset. This should reply with an estimated count, so that the
   // final results can be requested later.
   MC2_TEST_CHECK( maxCount( LAST_OFFSET - PAGE_SIZE, LAST_OFFSET - 1,
                             PAGE_SIZE,
                             320,
                             LAST_OFFSET, PAGE_SIZE ) == 320 );

   // If we request a page offset way outside the last offset we
   // should expext either estimated result count if it is less than
   // last offset, or we would expect the last offset.
   MC2_TEST_CHECK( maxCount( 1000, 1100,
                             0,
                             LAST_OFFSET - PAGE_SIZE/2,
                             LAST_OFFSET, PAGE_SIZE ) ==
                   LAST_OFFSET - PAGE_SIZE/2 );

   MC2_TEST_CHECK( maxCount( 1000, 1100,
                             0,
                             800,
                             LAST_OFFSET, PAGE_SIZE ) ==
                   LAST_OFFSET );
}

