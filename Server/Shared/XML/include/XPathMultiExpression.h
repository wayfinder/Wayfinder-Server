/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLTOOL_XPATH_MULTIEXPRESSION_H
#define XMLTOOL_XPATH_MULTIEXPRESSION_H

#include "config.h"

#include "MC2String.h"
#include "XPathExpression.h"

#include "PathTypes.h"

#include <vector>
#include <map>
#include <list>
#include <set>
#include <memory>
#include <iosfwd>


namespace Path {

class NameNode;

} // Path

namespace XMLTool {
namespace XPath {

/**
 * Evaluates multiple expressions in XPath style and calls
 * the associated NodeEvaluator for each node it finds.
 * Note: Only use the multiplier '*' on the first node. For example:
 * \verbatim
   /result_yp/adverts/@count
   /result_yp/adverts/@hits
   /result_yp/adverts/advert*
   /result_yp/adverts/advert/company_info/company_name
   /result_yp/adverts/advert/company_info/email
   \endverbatim
 * Notice how the '*' operator is not included for advert before company_info.
 * An attribute is declared as:
 * /result_yp/adverts/@count
 * This will try to find attribute \p count in node \p adverts . Attributes do not
 * have a multiplier option.
 * The evaluation can also handle namespace, such as "namespace:nodename".
 *
 * Example usage:
 * \code
 *
 * class Eval: public NodeEvaluator {
 * public:
 *    void operator() ( const DOMNode* node ) { ... }
 * };
 *
 * NodeDescription desc[] {
 *   { "/root/child*", new Eval() },
 *   { "/root/child/value2", new Eval() },
 * };
 *
 * MultiExpression exp( MultiExpression::
 *                      Description( desc, desc +
 *                                   sizeof( desc ) / sizeof( desc[ 0 ] ) ) ); 
 *
 * 
 * DOMNode* root = ... parse xml document ... ;
 * exp.evaluate( root );
 *
 * \endcode
 */
class MultiExpression {
public:
   typedef Path::NodeEvaluator< const DOMNode* > NodeEvaluator;
   typedef Path::NodeDescription< const DOMNode* > NodeDescription;

   /** 
    * vector of NodeDescriptions, only to be used to initialize 
    * MultiExpression
    */
   typedef vector<NodeDescription> Description;

   /**
    * Generates an evaluation tree to be used for evaluating nodes
    * The m_evaluator in NodeDescription will be owned and deleted by this
    * instance.
    * The param nspace sets the namespace prefix to be used when comparing 
    * nodes.
    * This param will be used as "namespace:node" comparision with the
    * expressions "node" name. For example: expression "/first/second" 
    * with namespace "ab" will be evaluated as "/ab:first/ab:second" .
    *
    * @param nodes Expressed as vector of NodeDescriptions.
    * @param nspace The namespace name.
    * @param removeNamespace If to remove namespaces from DOMElements when
    *                        comparing paths with nodes in a DOMDocument.
    *                        This might lead to problems if namespaces are
    *                        used to sperarate different nodes with the same
    *                        name.
    */
   explicit MultiExpression( const Description& nodes, 
                             const MC2String& nspace = MC2String(),
                             bool removeNamespace = false );

   ~MultiExpression();

   /**

    * This is a slow operation since it will traverse the tree and prefix
    * each node with the namespace. Set namespace
    * @param name the namespace name.
    */
   void setNamespace( const MC2String& name );

   /**
    * Evaluates the expression tree on the specified node.
    * 
    * @param root the root node to evaluate on.
    */
   void evaluate( const DOMNode* root ) const;

   /**
    * Prints the expression tree.
    * An attribute is printed as @name=? as a child node to the actual 
    * node it should be evaluated on.
    */
   friend ostream& operator << ( ostream& ostr, const MultiExpression& expr );

private:


   /**
    * @param parent parent to the root ( root can be null )
    * @param root the root node to evaluate name node in.
    * @param node the node for which children should be evaluated in root
    */
   void evaluate( const DOMNode* parent, 
                  const DOMNode* root, const Path::NameNode* node ) const;

   /// maps name node pointer to a node evaluator functor
   typedef map< const Path::NameNode*, NodeEvaluator* > EvaluatorMap;

   EvaluatorMap m_functions; ///< maps name node to node evaluator functor

   auto_ptr<Path::NameNode> m_rootNode; ///< root of the evaluator tree

   /// If to remove namespace before comparing nodes.
   bool m_removeNamespace;
};

}

}

#endif // XMLTOOL_XPATH_MULTIEXPRESSION_H
