/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// Tests so GET_STRING_* in ItemTypes is working
// properly.
//

#include "MC2UnitTestMain.h"

#include "ItemTypes.h"
#include "LangTypes.h"

MC2_UNIT_TEST_FUNCTION( stringIndexTest ) {
   uint32 testValue = ~0; // all bits set

   // make sure all values are set when using get
   MC2_TEST_CHECK( (uint32)GET_STRING_LANGUAGE( testValue ) ==
                   ( STRING_LANGUAGE_MASK >> STRING_LANGUAGE_SHIFT ) );

   MC2_TEST_CHECK( GET_STRING_TYPE( testValue ) ==
                   ( STRING_TYPE_MASK >> STRING_TYPE_SHIFT ) );

   MC2_TEST_CHECK( GET_STRING_INDEX( testValue ) ==
                   ( STRING_INDEX_MASK >> STRING_INDEX_SHIFT ) );

   // All masks should cover the entire 32 bits
   // and there should not be any overlapping
   // So test all permutations, a simple test with
   // A xor B xor C will not do, since A xor B might set one bit
   // and then be canceled by xor C.
   MC2_TEST_CHECK( (STRING_TYPE_MASK ^ STRING_LANGUAGE_MASK)
                   == ~(GET_STRING_INDEX( testValue ) 
                        << STRING_INDEX_SHIFT) );

   MC2_TEST_CHECK( (STRING_TYPE_MASK ^ STRING_INDEX_MASK)
                   == ~(GET_STRING_LANGUAGE( testValue ) 
                        << STRING_LANGUAGE_SHIFT) );

   MC2_TEST_CHECK( ItemTypes::name_t(STRING_LANGUAGE_MASK ^ STRING_INDEX_MASK)
                   == ~(GET_STRING_TYPE( testValue ) 
                        << STRING_TYPE_SHIFT) );

   MC2_TEST_CHECK( (STRING_LANGUAGE_MASK ^ STRING_TYPE_MASK ^ STRING_INDEX_MASK)
                   == testValue );

   uint32 nbrBitsForLanguage = 32 - STRING_LANGUAGE_SHIFT;
   uint32 nbrBitsForType = STRING_LANGUAGE_SHIFT - STRING_TYPE_SHIFT;
   uint32 nbrBitsForStringIndex = 32 - nbrBitsForLanguage - nbrBitsForType;

   MC2_TEST_CHECK( ( 1 << nbrBitsForLanguage ) - 1 ==
                   GET_STRING_LANGUAGE( testValue ) );
   MC2_TEST_CHECK( ( 1 << nbrBitsForType ) - 1 ==
                   GET_STRING_TYPE( testValue ) );

   MC2_TEST_CHECK( (uint32( ( 1 << nbrBitsForStringIndex ) - 1 )) ==
                   GET_STRING_INDEX( testValue ) );

}
