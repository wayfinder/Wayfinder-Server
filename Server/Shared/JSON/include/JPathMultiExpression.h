/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2JSON_JPATH_MULTI_EXPRESSION_H
#define MC2JSON_JPATH_MULTI_EXPRESSION_H

#include "config.h"
#include "MC2String.h"

#include "PathTypes.h"

#include <json_spirit/json_spirit_value.h>
#include <vector>
#include <map>


namespace Path {
class NameNode;
}


namespace MC2JSON {

namespace JPath {

// The type of the nodes
typedef json_spirit::Value NodeType;

/**
 * Exception thrown by evaluators.
 */
class EvaluateException: public std::exception {
public:
   /// @param what Extra exception information.
   EvaluateException( const MC2String& what ):
      m_what( what ) {
   }

   ~EvaluateException() throw() {
   }

   /// @return extra exception information.
   const char* what() const throw() { return m_what.c_str(); }

private:
   /// Extra exception information.
   MC2String m_what;
};

/**
 * Evaluates multiple expressions in XPath style and calls
 * the associated NodeEvaluator for each node it finds.
 *
 * Example usage:
 * \code
 *
 * class Eval: public MultiExpression::NodeEvaluator {
 * public:
 *    void operator() ( const NodeType& value ) {
 *      // ...
 *    }
 * };
 *
 * NodeDescription desc[] = {
 *   { "/root/child", new Eval() },
 *   { "/root/child/value2", new Eval() },
 * };
 *
 * MultiExpression exp( MultiExpression::
 *                      Description( desc, desc +
 *                                   sizeof( desc ) / sizeof( desc[ 0 ] ) ) );
 *
 *
 * DOMNode* root =  ... parse xml document ... ;
 * exp.evaluate( root );
 *
 * \endcode
 * This will evaluate the following JSON document:
 * \verbatim
 * { "root" : { "child" : { "value2" : "value_of_value" } } }
 * \endverbatim
 *
 * @see http://www.json.org/
 *
 */
class MultiExpression {
public:

   /// A single node description type.
   typedef Path::NodeDescription< const NodeType& > NodeDescription;

   /**
    * vector of NodeDescriptions, only to be used to initialize
    * MultiExpression
    */
   typedef vector< NodeDescription > Description;

   /// The evaluator type for this.
   typedef Path::NodeEvaluator< const NodeType& > NodeEvaluator;

   /**
    * Generates an evaluation tree to be used for evaluating nodes in
    * a JSON document.
    *
    * @param nodes expressed as vector of NodeDescriptions
    */
   explicit MultiExpression( const Description& nodes );

   ~MultiExpression();

   /**
    * Evaluates the expression tree on the specified node.
    *
    * @param root the root node to evaluate on.
    */
   void evaluate( const NodeType& root ) const;

private:

   /**
    * Evaluate next path node.
    * @param root Current parsed node.
    * @param node Current path node.
    */
   void evaluate( const NodeType& root, const Path::NameNode* node ) const;

   /**
    * Evaluates a json grammar object.
    * If the root in \c evaluate is an object type then this will be called.
    *
    * @param root JSON grammar object.
    * @param node Current node.
    */
   void evaluateObject( const NodeType& root,
                        const Path::NameNode* node ) const;

   /// maps name node pointer to a node evaluator functor
   typedef map< const Path::NameNode*, NodeEvaluator* > EvaluatorMap;

   EvaluatorMap m_functions; ///< maps name node to node evaluator functor

   auto_ptr< Path::NameNode > m_rootNode; ///< root of the evaluator tree
};

} // JPath

} // MC2JSON

#endif // MC2JSON_JPATH_MULTI_EXPRESSION_H
