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
#include "PropertyHelper.h"

/**
 *  Tests the getMostSpecializedProperty function.
 */
MC2_UNIT_TEST_FUNCTION( testGetMostSpecializedProperty ) {
   Properties::setPropertyFileName( "/dev/null" );

   Properties::insertProperty( "FOO",       "1" );

   Properties::insertProperty( "BAR",       "10" );
   Properties::insertProperty( "BAR_0",     "11" );

   Properties::insertProperty( "BAZ_1",     "20" );
   Properties::insertProperty( "MAP_BAZ",   "21" );
   
   Properties::insertProperty( "QWE",       "30" );
   Properties::insertProperty( "MAP_QWE_0", "31" );
   
   Properties::insertProperty( "ASD",       "40" );
   Properties::insertProperty( "ASD_0",     "41" );
   Properties::insertProperty( "MAP_ASD",   "42" );
   Properties::insertProperty( "MAP_ASD_0", "43" );

   using PropertyHelper::getMostSpecializedProperty;
   MC2_TEST_CHECK( 
      getMostSpecializedProperty<uint32>( "GURKA", "MAP", 0, 17 ) == 17 );

   MC2_TEST_CHECK(
      getMostSpecializedProperty<uint32>( "FOO", "MAP", 0, 17 ) == 1 );

   MC2_TEST_CHECK(
      getMostSpecializedProperty<uint32>( "BAR", "MAP", 0, 17 ) == 11 );
   MC2_TEST_CHECK(
      getMostSpecializedProperty<uint32>( "BAR", "MAP", 1, 17 ) == 10 );

   MC2_TEST_CHECK(
      getMostSpecializedProperty<uint32>( "BAZ", "MAP", 1, 17 ) == 21 );

   MC2_TEST_CHECK(
      getMostSpecializedProperty<uint32>( "QWE", "MAP", 0, 17 ) == 31 );

   MC2_TEST_CHECK(
      getMostSpecializedProperty<uint32>( "ASD", "MAP", 0, 17 ) == 43 );
   MC2_TEST_CHECK(
      getMostSpecializedProperty<uint32>( "ASD", "INFO", 0, 17 ) == 41 );
}
