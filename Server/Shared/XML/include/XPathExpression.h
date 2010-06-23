/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLTOOL_XPATH_EXPRESSION_H
#define XMLTOOL_XPATH_EXPRESSION_H

#include "config.h"

#include "MC2String.h"
#include <vector>

#include "XMLUtility.h"

/// \namespace Contains userful XML tools
namespace XMLTool {

/// \namespace Contains XPath expression evaluators and utilities
namespace XPath {

/**
 * Describes a XPath expression that can be evaluated
 * on any node. 
 * This was implemented because we do not have XPath in
 * xercesc on all machines.
 *
 * Example: 
 * Expression( "/rootnode/value" );
 * Will create an expression to match one hit for
 * value 1 in the following xml:
 * \verbatim
     <rootnode>
       <value> 1 </value>
       <value> 2 </value>
     </rootnode>
   \endverbatim   
 * For example expression:
 * \code
 *   Expression exp( "/rootnode/value*" );
 * \endcode
 * will match both valule 1 and 2 in previous xml.
 *
 * Example code:
 * \code
 * XPath::Expression exp( "/rootnode/value*" );
 * XPath::Expression::result_type valueNodes;
 * 
 * DOMNode* root = ... parse xml document ...;
 * 
 * valueNodes = exp.evaluate( root );
 * if ( ! valueNodes.empty() ) {
 *   ... do stuff with valueNodes ...
 * }
 * \endcode
 */
class Expression {
public:
   /**
    * Creates XPath expression from a string.
    * The nspace argument is appended to all sub nodes names, i.e
    * "/a/b/c" will be, with namespace "ns", "/ns:a/ns:b/ns:c"
    * @param expression a valid expression string.
    * @param nspace the namespace name
    */
   explicit Expression( const MC2String& expression,
                        const MC2String& nspace = MC2String() );

   ~Expression();

   /**
    * @return expression composed as a string
    */
   MC2String getExpression() const;

   /// result type from evaluate
   typedef std::vector<const DOMNode*> result_type;

   /**
    * Evaluate expression
    * @param node the node to evaluate expression in
    * @return a vector of nodes matching the expression
    */
   result_type evaluate( const DOMNode* node ) const;

private:
   /**
    * @param parent the parent node to param node
    * @param node node to match expression to
    * @param nodes nodes that matches the expression
    * @param depth the expression depth currently beeing parsed
    * @param nbrHits the number of hits left to find
    */
   void evaluate( const DOMNode* parent,
                  const DOMNode* node, 
                  result_type& nodes, 
                  uint32 depth,
                  int32& nbrHits ) const;

   MC2String m_expression; ///< the main expression
   std::vector<XMLCh*> m_nodeNames; ///< node names in depth order
   bool m_root; ///< true if expression is a root expression (not yet supported)
   bool m_attribute; ///< true if expression is an attribute 
   int32 m_nbrHits; ///< number of hits this expression must have, -1=unlimited
   const XMLCh* m_backStr; ///< value of ".." in ucs
};

/**
 * For each expression in root it finds nodes; call op
 * @param exp the expression to evaluate
 * @param root the node to evaluate expression on
 * @param op functor to call for each node that was found.
 */
template <typename T>
void for_each( const Expression& exp, const DOMNode* root,
               T& op ) {
   vector<const DOMNode*> nodes( exp.evaluate( root->getFirstChild() ) );
   for ( uint32 i = 0; i < nodes.size(); ++i ) {
      op( nodes[ i ] );
   }
}

}
}

#endif // XMLTOOL_XPATH_EXPRESSION_H
