/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLTOOL_XPATH_SUBASSIGNER_H
#define XMLTOOL_XPATH_SUBASSIGNER_H

#include "XPathMultiExpression.h"

#include <vector>

namespace XMLTool {

namespace XPath {
/**
 * Handles sub expressions with multiple values.
 *
 */
template <typename T>
class SubAssigner: public MultiExpression::NodeEvaluator {
public:
   typedef std::vector<T> TypeArray;
   SubAssigner( TypeArray& values ):
      m_expr( NULL ),
      m_values( values ),
      m_tempValue() {
      buildExpression();
   }

   ~SubAssigner() {
      delete m_expr;
   }

   /// @see MultiExpression::NodeEvaluator
   void operator()( const DOMNode* node ) {
      m_tempValue = T(); // clear old values
      // fetch new value
      m_expr->evaluate( node );
      m_values.push_back( m_tempValue );
   }

private:
   /// Specialize this and use setExpression for the final expression.
   void buildExpression();

   /// Use this to get the temporary value
   T& getValue() {
      return m_tempValue;
   }
   /// Sets the expression to use
   void setExpression( MultiExpression::NodeDescription* desc,
                       size_t size ) {
      // might throw in the future.

      MultiExpression* expr =
         new MultiExpression( MultiExpression::
                              Description( desc, desc + size ) );
      // a new expression is ready
      delete m_expr;
      m_expr = expr;
   }
   /// XML Evaluator
   MultiExpression* m_expr;
   /// Assign all the parsed values to this.
   TypeArray& m_values;
   /// Temporary value to use while parsing.
   T m_tempValue;
};

/// Helper to create sub assigners.
template <typename T>
SubAssigner<typename T::value_type>* makeSubAssigner( T& values ) {
   return new SubAssigner<typename T::value_type>( values );
}

} // XPath

} // XMLTool

#endif //XMLTOOL_XPATH_SUBASSIGNER_H
