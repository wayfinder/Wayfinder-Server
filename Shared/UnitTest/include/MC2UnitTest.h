/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2UNITTEST_H
#define MC2UNITTEST_H

#include "MC2UnitTestData.h"

#include "boost/lexical_cast.hpp"

/**
 * \namespace contains unit test functions.
 *  Use:
 *  Use MC2_UNIT_TEST_FUNCTION to declare a test-function.
 *  Use MC2_TEST_REQUIRED to test a requirement, this will end the test function if
 *  it failes and continue to test next test-function.
 *  Use MC2_TEST_CHECK and MC2_TEST_CHECK_EXT tests conditions.
 *
 *  Usage:
 *  \code
 *  #include "MC2UnitTestMain.h"
 *
 *  MC2_UNIT_TEST_FUNCTION( testData ) {
 *     int value = 0;
 *     MC2_TEST_CHECK( value == 0 );
 *     value++;
 *     MC2_TEST_REQUIRED( value == 1 );
 *  }
 *  \endcode
 *
 *  If the tests are spread out in different files then include MC2UnitTest.h
 *  in them and in the last file include MC2UnitTestMain.h.
 */
namespace MC2UnitTest {

/// Unit test function signature
typedef void (*UnitTestFunction)();

class UnitTestClass;

/// Exception thrown when a required test failed.
/// @see MC2_TEST_REQUIRED
/// @see MC2_TEST_REQUIRED_EXT
class Exception: public std::exception {
public:
   Exception() throw() {
   }

   ~Exception() throw() {
   }

   const char *what() const throw() {
      return "UnitTest: Failed requirements.";
   }


};

/// Main class that holds all unit test functions.
class Functions {
public:
   /// Register unit test function, use the MC2_UNIT_TEST_FUNCTION !
   static void registerTest( UnitTestClass& func );

   /// @returns False if the tests succeeded.
   static bool run();
   /// @return test data
   static TestData& getData() { return m_data; }

private:
   typedef std::vector<UnitTestClass*> Tests;
   static Tests m_functions; ///< holds all test functions
   static TestData m_data; ///< holds the test data, failures etc.
};

/// A unit test class that holds the real test function, used by \c Functions
struct UnitTestClass {
public:
   UnitTestClass( UnitTestFunction func,
                  const char* name ):
      m_func( func ),
      m_name( name ) {
      Functions::registerTest( *this );
   }

   /// Calls the test function.
   void run() {
      (*m_func)();
   }

   /// @return function name thats beeing called by \c run
   const MC2String& getName() const {
      return m_name;
   }

private:
   UnitTestFunction m_func; ///< Test function
   MC2String m_name; ///< Name of test function.
};

/// Use this to auto register unit test function
/// @param func The function to call when testing. @see UnitTestFunction
#define MC2_UNIT_TEST_FUNCTION( func ) \
void func(); \
static MC2UnitTest::UnitTestClass UnitTestClass_##func( func, #func ); \
void func()

/// Test a requirement, use this test to end the entire test function if it
/// failes.
/// @param requirement The requirement to test.
#define MC2_TEST_REQUIRED( requirement ) \
  MC2UnitTest::Functions::getData().test(); \
  if ( ! ( requirement ) ) { \
     MC2UnitTest::Functions::getData().addFailure( #requirement, __FILE__, \
                                                   __FUNCTION__, __LINE__ ); \
     throw MC2UnitTest::Exception(); \
  }

/// Test a requirement and include some extended info, use this test to end the
/// entire test function if it failes.
/// @param requirement The requirement to test
/// @param ext Extended info to show when it failes.
#define MC2_TEST_REQUIRED_EXT( requirement, ext ) \
  MC2UnitTest::Functions::getData().test(); \
  if ( ! ( requirement ) ) { \
     MC2UnitTest::Functions::getData().addFailure( #requirement, __FILE__, \
                                                   __FUNCTION__, __LINE__, \
                                                   ext ); \
     throw MC2UnitTest::Exception(); \
  }

/// Test a condition and add it to failure if it failes.
/// @param condition The condition to test.
#define MC2_TEST_CHECK( condition ) \
   MC2UnitTest::Functions::getData().test(); \
   if ( ! ( condition ) ) { \
      MC2UnitTest::Functions::getData(). \
         addFailure( #condition, __FILE__, __FUNCTION__, __LINE__ ); \
   }

/// Test a condition and add it to failure if it failes and include some
/// extended info.
/// @param condition The condition to test.
/// @param ext Extended info to show when it failes.
#define MC2_TEST_CHECK_EXT( condition, ext ) \
   MC2UnitTest::Functions::getData().test(); \
   if ( ! ( condition ) ) { \
       MC2UnitTest::Functions::getData(). \
          addFailure( #condition, __FILE__, __FUNCTION__, __LINE__, ext ); \
   }


} // MC2UnitTest

#endif // MC2UNITTEST_H
