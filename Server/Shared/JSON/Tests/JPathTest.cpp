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

#include "JPathMultiExpression.h"
#include "JPathAssigner.h"

#include <json_spirit/json_spirit_reader_template.h>

//
// This is a test for JPath evaluation.
//


using namespace MC2JSON;
using namespace JPath;

/// Compares a predefined value to a parsed value.
struct TestExpression: public MultiExpression::NodeEvaluator {
   /// @param value Compare to this value.
   TestExpression( const MC2String& value ):
      m_value( value ),
      m_wasEvaluated( false )  {
   }

   ~TestExpression() {
      // all expressions must be evaluated.
      MC2_TEST_CHECK_EXT( m_wasEvaluated, m_value );
   }

   void operator()( const NodeType& value ) {
      MC2_TEST_CHECK_EXT( value.get_str() == m_value,
                          m_value );
      m_wasEvaluated = true;
   }

   MC2String m_value; ///< expected value
   bool m_wasEvaluated; ///< whether this expression was evaluated.
};

template< typename T >
struct TestExpr: public MultiExpression::NodeEvaluator {
   /// @param value Compare to this value.
   TestExpr( T value ):
      m_value( value ),
      m_wasEvaluated( false )  {
   }

   ~TestExpr() {
      // all expressions must be evaluated.
      MC2_TEST_CHECK( m_wasEvaluated );
   }

   void operator()( const NodeType& value ) {
      MC2_TEST_CHECK( value.get_value< T >() == m_value );
      m_wasEvaluated = true;
   }

   T m_value; ///< expected value
   bool m_wasEvaluated; ///< whether this expression was evaluated.
};

struct PrintExpr: public MultiExpression::NodeEvaluator {
   void operator()( const NodeType& value ) {
      mc2dbg << "I am here." << endl;
      MC2String str = value.get_str();
      mc2dbg << "Value is: " << str << endl;
   }
};

MC2_UNIT_TEST_FUNCTION( jpathTest ) {
   // setup expression
   MultiExpression::Description desc;
   vector< MC2String > testStrArray;
   testStrArray.push_back( "value_1" );
   testStrArray.push_back( "value_2" );

   vector< MC2String > otherStrArray;

   vector< int > testIntArray;
   testIntArray.push_back( 66 );
   testIntArray.push_back( 89 );
   vector< int > otherIntArray;

   typedef MultiExpression::Description::value_type NodeDesc;
   desc.push_back( (NodeDesc){ "a/b/c", new TestExpression( "the_c" ) } );
   desc.push_back( (NodeDesc){ "a/b/d", new TestExpression( "the_d" ) } );
   desc.push_back( (NodeDesc){ "e/f", new TestExpression( "the_f" ) } );
   desc.push_back( (NodeDesc){ "g", new TestExpression( "the_g" ) } );
//   desc.push_back( (NodeDesc){ "int", new TestExpr<int>( int(201) ) } );
   desc.push_back( (NodeDesc){ "d", new TestExpr<double>( double(2.0) ) } );
   desc.push_back( (NodeDesc){ "b", new TestExpr<bool>( true ) } );
   desc.push_back( (NodeDesc){ "vector", JPath::makeAssigner( otherStrArray ) } );
   desc.push_back( (NodeDesc){ "vector_int", JPath::makeAssigner( otherIntArray ) } );
   MultiExpression expr( desc );

   // data to evaluate
   MC2String jsonData( "{ \"a\" : { \"b\" : { \"c\" : \"the_c\", \"d\" : \
   \"the_d\" } },\"e\" : { \"f\" : \"the_f\" }, \"dont_readthis\" : \
   \"dontread\", \"g\" : \"the_g\" , \"int\":201, \"d\":2.0, \"b\":true, \
   \"vector\" :[\"value_1\", \"value_2\"], \"vector_int\" : [ 66, 89 ] }" );

   // parse and evaluate data
   NodeType value;
   MC2_TEST_REQUIRED( json_spirit::read_string( jsonData, 
                                                value ) );
   expr.evaluate( value );

   MC2_TEST_CHECK( otherStrArray == testStrArray );
   MC2_TEST_CHECK( otherIntArray == testIntArray );
}
