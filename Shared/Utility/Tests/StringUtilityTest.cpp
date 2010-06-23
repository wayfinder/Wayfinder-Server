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
#include "StringUtility.h"
#include "ScopedArray.h"

/**
 * Turns foo into "foo".
 */
MC2String quote( const MC2String& str ) {
   return "\"" + str + "\"";
}

/**
 * Tests StringUtility::trimEnd.
 */
MC2_UNIT_TEST_FUNCTION( testTrimEnd ) {
   const char* testData[][2] = {
      // in-data      result
      { "",           "" },
      { "abc",        "abc" },
      { "abc ",       "abc" },
      { " abc ",      " abc" },
      { "abc     ",   "abc" },
      { "abc\t \n",   "abc" },
      { "a b c",      "a b c" },
      { "a b c ",     "a b c" }
   };

   for ( size_t i = 0; i < sizeof( testData )/sizeof( testData[0] ); ++i ) {
      ScopedArray<char> str( StringUtility::newStrDup( testData[i][0] ) );
      StringUtility::trimEnd( str.get() );
      MC2_TEST_CHECK_EXT( strcmp( str.get(),
                                  testData[i][1] ) == 0,
                          quote( testData[i][0] ) );
   }
}

/**
 * Tests StringUtility::trimEnd with the additional parameter.
 */
MC2_UNIT_TEST_FUNCTION( testTrimEndAdditional ) {
   const char* testData[][3] = {
      // in-data      result     additional
      { "",           "",        "," },
      { "abc,",       "abc",     "," },
      { "abc ,",      "abc",     "," },
      { "abc..., .",  "abc",     ",."},
      { ", a b,c ,",  ", a b,c", "," }
   };

   for ( size_t i = 0; i < sizeof( testData )/sizeof( testData[0] ); ++i ) {
      ScopedArray<char> str( StringUtility::newStrDup( testData[i][0] ) );
      StringUtility::trimEnd( str.get(), testData[i][2] );
      MC2_TEST_CHECK_EXT( strcmp( str.get(),
                                  testData[i][1] ) == 0,
                          quote( testData[i][0] ) );
   }
}

/**
 * Tests StringUtility::trimStart.
 */
MC2_UNIT_TEST_FUNCTION( testTrimStart ) {
   const char* testData[][2] = {
      // in-data      result
      { "",           "" },
      { "abc",        "abc" },
      { " abc",       "abc" },
      { " abc ",      "abc " },
      { "    abc",    "abc" },
      { " \t \f abc", "abc" },
      { "a b c",      "a b c" },
      { " a b c",     "a b c" }
   };

   for ( size_t i = 0; i < sizeof( testData )/sizeof( testData[0] ); ++i ) {
      MC2_TEST_CHECK_EXT( strcmp( StringUtility::trimStart( testData[i][0] ),
                                  testData[i][1] ) == 0,
                          quote( testData[i][0] ) );
   }
}

/**
 * Tests StringUtility::trimStart with the additional parameter.
 */
MC2_UNIT_TEST_FUNCTION( testTrimStartAdditional ) {
   const char* testData[][3] = {
      // in-data            result           additional
      { "",                 "",              "," },
      { ",abc",             "abc",           "," },
      { ", abc",            "abc",           "," },
      { "..., .abc..., .",  "abc..., .",     ",."},
      { ", a b,c ,",        "a b,c ,",       "," }
   };

   for ( size_t i = 0; i < sizeof( testData )/sizeof( testData[0] ); ++i ) {
      char* trimmed = StringUtility::trimStart( testData[i][0],
                                                testData[i][2] );
      MC2_TEST_CHECK_EXT( strcmp( trimmed, testData[i][1] ) == 0,
                          quote( testData[i][0] ) );
   }
}

/**
 * Tests StringUtility::trimStartEnd.
 */
MC2_UNIT_TEST_FUNCTION( testTrimStartEnd ) {
   const char* testData[][2] = {
      // in-data           result
      { "",                "" },
      { "abc",             "abc" },
      { " abc",            "abc" },
      { "abc ",            "abc" },
      { " abc ",           "abc" },
      { "   abc    ",      "abc" },
      { " \t a b c \n",    "a b c" }
   };

   for ( size_t i = 0; i < sizeof( testData )/sizeof( testData[0] ); ++i ) {
      MC2_TEST_CHECK_EXT( StringUtility::trimStartEnd( testData[i][0] ) ==
                          MC2String( testData[i][1] ),
                          quote( testData[i][0] ) );
                          
   }
}

/**
 * Tests StringUtility::trimStartEnd with the additional parameter.
 */
MC2_UNIT_TEST_FUNCTION( testTrimStartEndAdditional ) {
   const char* testData[][3] = {
      // in-data            result           additional
      { "",                 "",              "," },
      { ",abc",             "abc",           "," },
      { ", abc",            "abc",           "," },
      { "..., .abc..., .",  "abc",           ",."},
      { ", a b,c ,",        "a b,c",         "," },
      { ".,abc.,",          ",abc.,",        "." }
   };
   
   for ( size_t i = 0; i < sizeof( testData )/sizeof( testData[0] ); ++i ) {
      MC2String trimmed = StringUtility::trimStartEnd( testData[i][0], 
                                                       testData[i][2] );
      MC2_TEST_CHECK_EXT( trimmed == MC2String( testData[i][1] ),
                          quote( testData[i][0] ) );
                          
   }
}
