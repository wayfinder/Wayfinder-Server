/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TESTDATA_H
#define TESTDATA_H

#include "config.h"
#include "MC2String.h"

#include "boost/lexical_cast.hpp"

#include <iostream>
#include <exception>

namespace MC2UnitTest {
/**
 * Holds number of tests and the failures.
 */
class TestData {
public:
   /// Information about a failure.
   /// What, where, and extra data.
   struct Failure {
      Failure( const char* condition,
               const char* file,
               const char* function,
               int line,
               const MC2String& extra = MC2String() ):
         m_condition( condition ),
         m_file( file ),
         m_function( function ),
         m_extraData( extra ),
         m_line( line ) {
      }
      Failure():m_line( 0 ) { }
      MC2String m_condition;
      MC2String m_file;
      MC2String m_function;
      MC2String m_extraData;
      int m_line;
   };

   typedef uint32 SizeType;

   TestData():m_nbrTests( 0 ) { }

   /// Increase the number of tests
   void test() {
      m_nbrTests++;
   }

   /// Adds a failure with basic data.
   void addFailure( const char* condition, const char* file, const char* function, int line ) {
      m_failures.push_back( Failure( condition, file, function, line ) );
   }

   /// Adds a failure with basic data and including an extra generic data type.
   template <typename T>
   void addFailure( const char* condition, const char* file, const char*
                    function, int line, const T& data ) {
      m_failures.push_back( Failure( condition, file, function, line,
                                     boost::lexical_cast<MC2String>
                                     ( data ) ) );
   }

   /// @return Number of tests.
   uint32 getNbrTests() const {
      return m_nbrTests;
   }

   /// @return Number of failures
   SizeType getNbrFailures() const {
      return m_failures.size();
   }

   /// @return Failure number \c i
   const Failure& getFailure( SizeType i ) const {
      return m_failures.at( i );
   }

private:
   uint32 m_nbrTests; ///< Number of tests done.
   typedef std::vector<Failure> FailureVector;
   FailureVector m_failures;
};

} // MC2UnitTest

std::ostream& operator << ( std::ostream& ostr,
                            const MC2UnitTest::TestData::Failure& failure );

std::ostream& operator << ( std::ostream& ostr, const MC2UnitTest::TestData& data );

#endif //  TESTDATA_H
