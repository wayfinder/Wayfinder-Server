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

#include "Path.h"

#include <map>

struct IntEvaluator: public Path::NodeEvaluator< int > {
   IntEvaluator( int initialValue ):
      m_value( initialValue ) {
   }

   void operator()( int i ) {
      m_value = i;
   }

   int m_value;
};

void testNode( const Path::NameNode& node,
               const char* name,
               int numberOfSubElements ) {
   MC2_TEST_REQUIRED( node.getName() == name );
   MC2_TEST_REQUIRED( std::distance( node.begin(), node.end() ) ==
                      numberOfSubElements );

   if ( numberOfSubElements > 0 ) {
      MC2_TEST_REQUIRED( ! node.empty() );
   } else {
      MC2_TEST_REQUIRED( node.empty() );
   }
}

typedef std::map< Path::NameNode*, const Path::NodeEvaluator< int >* > EvaluatorMap;
void testTree( const Path::NameNode& rootNode );
void testEvalMap( const Path::NameNode& rootNode,
                  EvaluatorMap& evalMap );

MC2_UNIT_TEST_FUNCTION( pathTest ) {
   // setup types
   typedef Path::NodeDescription< int > IntDescription;
   typedef vector< IntDescription > Descriptions;

   // setup variables
   Path::NameNode rootNode( "eval" );
   EvaluatorMap evalMap;
   Descriptions desc;
   // Setup a tree that looks like this:
   //         .__ root node_____.
   //        /         |     \   \.
   //       a _.       b     e    g
   //      / \  \      |     |
   //     b   c  g     c     f
   //    / \   \       |
   //   c   d   d      d
   //
   desc.push_back( (IntDescription){ "a/b/c", new IntEvaluator( 1 ) } );
   desc.push_back( (IntDescription){ "a/b/d", new IntEvaluator( 2 ) } );
   desc.push_back( (IntDescription){ "a/c/d", new IntEvaluator( 3 ) } );
   desc.push_back( (IntDescription){ "a/g", new IntEvaluator( 4 ) } );
   desc.push_back( (IntDescription){ "b/c/d", new IntEvaluator( 5 ) } );
   desc.push_back( (IntDescription){ "e/f", new IntEvaluator( 6 ) } );
   desc.push_back( (IntDescription){ "g", new IntEvaluator( 7 ) } );
   Path::createNodeTree( rootNode, evalMap,
                         desc, // description
                         "" ); // no namespace
   testTree( rootNode );
   testEvalMap( rootNode, evalMap );
}

void testTree( const Path::NameNode& rootNode ) {
   // first, the rootNode should have four children:
   // a, b, e and g
   testNode( rootNode, "eval", 4 );

   Path::NameNode::const_iterator it = rootNode.begin();
   MC2_TEST_REQUIRED( (*it)->getName() == "a" );
   ++it;
   MC2_TEST_REQUIRED( (*it)->getName() == "b" );
   ++it;
   MC2_TEST_REQUIRED( (*it)->getName() == "e" );
   ++it;
   MC2_TEST_REQUIRED( (*it)->getName() == "g" );

   //
   // Test the "a"-tree
   //
   it = rootNode.begin();
   // should have 3 elements
   testNode( **it, "a", 3 );

   //
   // test sub node of "b" of "a"
   //
   it = (*it)->begin();
   testNode( **it, "b", 2 );

   Path::NameNode::const_iterator subIt = (*it)->begin();
   // first subnode must be "c"
   testNode( **subIt, "c", 0 );

   ++subIt;
   // second subnode must be "d" and empty
   testNode( **subIt, "d", 0 );

   //
   // test sub node "c" of "a"
   //
   it = rootNode.begin();
   it = (*it)->begin();
   ++it;
   testNode( **it, "c", 1 );
   // the subnode's name should be "d" and empty
   testNode( *(*(*it)->begin()), "d", 0 );

   //
   // test sub node "g" of "a"
   //
   it = rootNode.begin();
   it = (*it)->begin();
   ++it;
   ++it;
   // "g" should be empty
   testNode( **it, "g", 0 );

   //
   // Now test the "b" tree of the root node
   //
   it = rootNode.begin();
   ++it;
   // "b" should have one element
   testNode( **it, "b", 1 );

   // test "c" of "b"
   it = (*it)->begin();
   // should also have one element
   testNode( **it, "c", 1 );

   // test "d" of "c" of "b"
   it = (*it)->begin();
   // it should be empty
   testNode( **it, "d", 0 );

   //
   // Now test "e" tree of the root node
   //
   it = rootNode.begin();
   ++it;
   ++it;
   testNode( **it, "e", 1 );

   // test "f" of "e" of root node
   it = (*it)->begin();
   testNode( **it, "f", 0 );

   it = rootNode.begin();
   ++it;
   ++it;
   ++it;
   testNode( **it, "g", 0 );

}

void testEval( Path::NameNode& node,
               EvaluatorMap& evalMap,
               int expectedValue ) {

   EvaluatorMap::const_iterator evalIt = evalMap.find( &node );
   MC2_TEST_REQUIRED( evalIt != evalMap.end() );
   // should be value '1' in the evaluator
   // dynamic cast with reference first, so it throws on failure,
   // we want to reuse eval.
   IntEvaluator* eval = const_cast< IntEvaluator*>
      ( &dynamic_cast< const IntEvaluator& >( *evalIt->second ) );
   MC2_TEST_REQUIRED( eval->m_value == expectedValue );
   eval->operator()( expectedValue * 10 );
   MC2_TEST_REQUIRED( eval->m_value == expectedValue * 10 );

}

void testEvalMap( const Path::NameNode& rootNode,
                  EvaluatorMap& evalMap ) {
   // Gather all the leaf nodes
   Path::NameNode::const_iterator it = rootNode.begin();
   // it = "a"
   it = (*it)->begin(); // "b" of "a"
   it = (*it)->begin(); // "c" of "b" of "a"
   MC2_TEST_REQUIRED( (*it)->getName() == "c" );
   testEval( **it, evalMap, 1 );
   it = rootNode.begin(); // "a"
   it = (*it)->begin(); // "b" of "a"
   it = (*it)->begin(); // "c" of "b" of "a"
   ++it; // "d" of "b" of "a"

   MC2_TEST_REQUIRED( (*it)->getName() == "d" );
   testEval( **it, evalMap, 2 );
   it = rootNode.begin(); // "a"
   it = (*it)->begin(); // "b" of "a"
   ++it; // "c" of "a"
   it = (*it)->begin(); // "d" of "c" of "a"
   MC2_TEST_REQUIRED( (*it)->getName() == "d" );
   testEval( **it, evalMap, 3 );

   it = rootNode.begin(); // "a"
   it = (*it)->begin(); // "b" of "a"
   ++it; // "c" of "a"
   ++it; // "g" of "a"
   MC2_TEST_REQUIRED( (*it)->getName() == "g" );
   testEval( **it, evalMap, 4 );

   it = rootNode.begin(); // "a"
   ++it; // "b"
   it = (*it)->begin(); // "c" of "b"
   it = (*it)->begin(); // "d" of "c" of "b"
   MC2_TEST_REQUIRED( (*it)->getName() == "d" );
   testEval( **it, evalMap, 5 );

   it = rootNode.begin(); // "a"
   ++it; // "b"
   ++it; // "e"
   it = (*it)->begin(); // "f"
   MC2_TEST_REQUIRED( (*it)->getName() == "f" );
   testEval( **it, evalMap, 6 );

   it = rootNode.begin(); // "a"
   ++it; // "b"
   ++it; // "e"
   ++it; // "g"
   MC2_TEST_REQUIRED( (*it)->getName() == "g" );
   testEval( **it, evalMap, 7 );
}
