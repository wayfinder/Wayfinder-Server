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
// This file contains test for UserPassword
//

#include "MC2UnitTestMain.h"

#include "UserPassword.h"
#include "STLUtility.h"

#include <ctype.h>

MC2_UNIT_TEST_FUNCTION( testPassword ) {
   const char* plainText = "berras_car_crusher";
   const char* seed = "99999";

   MC2String hashedPassword = UserPassword::create( plainText, seed );

   MC2_TEST_REQUIRED( ! hashedPassword.empty() );
   // make sure the hashedPassword does not contain any
   // plain text stuff from the original password
   MC2_TEST_REQUIRED( ! STLUtility::has( hashedPassword, plainText ) );
   MC2_TEST_REQUIRED( ! STLUtility::has( hashedPassword, seed ) );
   for ( size_t i = 0; i < hashedPassword.size(); ++i ) {
      // make sure the hashed password is printable,
      // it is suppose to be ready to be stored in a database.
      MC2_TEST_REQUIRED( isprint( hashedPassword[ i ] ) );
   }
   // now see if we can compare it 
   MC2_TEST_REQUIRED( UserPassword::compare( hashedPassword.c_str(),
                                             plainText, seed ) );

   // with seed and plainText reversed too, which should fail
   MC2_TEST_REQUIRED( ! UserPassword::compare( hashedPassword.c_str(),
                                               seed, plainText ) );

}
