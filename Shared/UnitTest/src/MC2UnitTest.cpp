/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2UnitTest.h"

MC2UnitTest::TestData MC2UnitTest::Functions::m_data;
MC2UnitTest::Functions::Tests MC2UnitTest::Functions::m_functions;

void MC2UnitTest::Functions::registerTest( UnitTestClass& func ) {
   m_functions.push_back( &func );
}

bool MC2UnitTest::Functions::run() {
   bool failed = false;
   for ( Tests::iterator it = m_functions.begin();
         it != m_functions.end(); ++it ) {
      try {
         (*it)->run();
      } catch ( const Exception& e ) {
         cout << e.what() << endl;
         failed = true;
      }
   }
   failed = failed || m_data.getNbrFailures() != 0;

   if ( failed ) {
      cout << "MC2UnitTest failed!" << endl;
   } else {
      cout << "MC2UnitTest succeeded!" << endl;
   }

   cout << MC2UnitTest::Functions::getData() << endl;

   return failed;
}
