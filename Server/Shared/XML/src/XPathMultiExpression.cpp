/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XPathMultiExpression.h"
#include "DeleteHelpers.h"
#include "StringUtility.h"
#include "XMLTool.h"

#include "Path.h"

namespace XMLTool {
namespace XPath {


MultiExpression::MultiExpression( const Description& nodes,
                                  const MC2String& nspace,
                                  bool removeNamespace )
      : m_rootNode( new Path::NameNode( "evaluation tree" ) ),
        m_removeNamespace( removeNamespace ) 
{
   // build tree
   Path::createNodeTree( *m_rootNode, m_functions, nodes, nspace );

}

MultiExpression::~MultiExpression() {
   STLUtility::deleteAllSecond( m_functions );
}

void MultiExpression::evaluate( const DOMNode* parent,
                                const DOMNode* root, 
                                const Path::NameNode* node ) const {
   //
   // If there is no children to the parent then
   // we check for attributes in the parent.
   //
   if ( root == NULL ) {
      // ok if root is null but node holds some values then
      // only check attribute values for node in parent
      for ( Path::NodeList::const_iterator it = node->begin();
            it != node->end(); ++it ) {
         if ( ! (*it)->isAttributeNode() ) {
            continue;
         }
         // get attribute node
         const DOMNode* attribNode = 
            findAttribConst( parent, (*it)->getName().c_str(), 
                             false ); // ignore case

         if ( attribNode != NULL ) { 
            EvaluatorMap::const_iterator eval = m_functions.find( *it );
            // no functor?, this is not good
            if ( eval == m_functions.end() ) { 
               mc2log << warn 
                      << "[XPath::ME] Could not find function for attrib node: "
                      << (*it)->getName() << endl;
               continue;
            }
            // check if we use the = operator for a attrib 
            if ( (*it)->isAttributeValueNode() ) {
               MC2String attribValue;
               attribValue = 
                  XMLUtility::transcodefrom( attribNode->getNodeValue() );
               if ( strcasecmp( (*it)->getAttributeValue().c_str(),
                        attribValue.c_str() ) != 0 ) {
                  // no match in value skip it.. take next nod.
                  continue;
               } else {
                  // call functor with the attrib node
                  const_cast<NodeEvaluator&>(*(*eval).second)( parent );
               }
            } else {
               // call functor with the attrib node
               const_cast<NodeEvaluator&>(*(*eval).second)( attribNode );
            }
         }
      }

      // attribute check done. continue with lower level nodes
      return;
   }

   // The algorithm is as follows:
   //
   // For each child node that is an element node do
   //   find the matching name node in the name node children
   //     if name node was found then 
   //        if it already has been checked and 
   //           is not a multi node( i.e in skip set )
   //           skip it
   //        else if it is a leaf node ( no children )
   //           if it is an attribute name node then
   //              find the attribute in the parent DOM node and call
   //              the corresponding functor with the DOM attrib node
   //              as argument
   //           else    
   //              call the corresponding functor with the current DOM node
   //              as argument
   //        else
   //           go (recursively )into name nodes childs and the 
   //           DOM nodes child and repeat until a leaf name node is found
   //        
   //
   // Note: for each leaf node there is a check for "multible node"-type.
   // A multiple node is a node that will be found and evaluated more than once.
   // A single node is only evaluated once and put in the skip set.



   // Holds nodes to skip when running evaluate, 
   // i.e if they are single hit nodes ( without '*')
   set<Path::NameNode*> skip;
   bool hasElementNodes = false;
   for ( const DOMNode* child = root;
         child != NULL; child = child->getNextSibling() ) {
      // only interested in element nodes, the attribute nodes
      // are taken care of inside these element nodes
      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }

      hasElementNodes = true;

      MC2String nodeName = XMLUtility::transcodefrom( child->getNodeName() );
      if ( m_removeNamespace ) {
         size_t colonPos = nodeName.find( ':' );
         if ( colonPos != MC2String::npos ) {
            // Prefixed by namespace
            nodeName.erase( 0, colonPos + 1 );
         }
      }

      for ( Path::NodeList::const_iterator it = node->begin();
            it != node->end(); ++it ) {

         // search skip nodes and then compare name
         if ( skip.find( *it ) != skip.end() ) {
            // found a name node which we should skip
            continue;
         } 

         // if node is attribute node then search for attribute in parent
         if ( (*it)->isAttributeNode() ) { 
            const DOMNode* attribNode = 
               findAttribConst( parent, (*it)->getName().c_str(), 
                                false ); // ignore case

            if ( attribNode != NULL ) { 
               EvaluatorMap::const_iterator eval = m_functions.find( *it );
               if ( eval == m_functions.end() ) { 
                  mc2log << warn 
                      << "[XPath::ME] Could not find function for attrib node: "
                      << (*it)->getName() << endl;
                  continue;
               }
               // check if we use the = operator for a attrib 
               if ( (*it)->isAttributeValueNode() ) {
                  MC2String attribValue;
                  attribValue = 
                     XMLUtility::transcodefrom( attribNode->getNodeValue() );
                  if ( strcasecmp( (*it)->getAttributeValue().c_str(),
                                   attribValue.c_str() ) != 0 ) {
                     // no match in value skip it.. take next nod.
                     continue;
                  } else {
                     // call functor with the attrib node
                     const_cast<NodeEvaluator&>(*(*eval).second)( parent );

                     skip.insert( *it );
                  }
               } else {
                  // call functor with the attrib node
                  const_cast<NodeEvaluator&>(*(*eval).second)( attribNode );

                  skip.insert( *it );
               }
            }

            // no more node tests needed, take next node.
            continue;
         }
         
         if ( strcasecmp( (*it)->getName().c_str(), nodeName.c_str() ) != 0 ) {
            continue;
         }
         // if node is empty then we have a leaf,
         // which means we should call the corresponding functor
         if ( (*it)->empty() ) {
            EvaluatorMap::const_iterator eval = m_functions.find( *it );
            if ( eval == m_functions.end() ) {
               mc2log << warn 
                      << "[XPath::ME] Could not find function for leaf node: "
                      << (*it)->getName() << endl;
            } else {
               // call functor with this node
               const_cast<NodeEvaluator&>(*(*eval).second)( child );
            }

         } else {
            // go deeper in to the tree
            // but first see if there is a functor for this node and call it
            EvaluatorMap::const_iterator eval = m_functions.find( *it );
            if ( eval != m_functions.end() ) {
               // call functor with this node
               const_cast<NodeEvaluator&>(*(*eval).second)( child );
            }
            // now we can go deeper
            evaluate( child, child->getFirstChild(), (*it) );
         }

         // if it is a single hit node, then we should skip it
         if ( ! (*it)->isMultiNode() ) {
            skip.insert( *it );
         }
      }
   }

   if ( ! hasElementNodes ) {
      // If the root does not have any valid element nodes
      // then we need to check the attribute nodes for
      // the current evaluation tree "node"
      evaluate( parent, NULL, node );
   }
}

void MultiExpression::evaluate( const DOMNode* root ) const {
   if ( root == NULL ) {
      return;
   }
   evaluate( root, root->getFirstChild(), m_rootNode.get() );
}

ostream& operator << ( ostream& ostr, const MultiExpression& expr ) {
   Path::Impl::printNode( ostr, *expr.m_rootNode );
   return ostr;
}

} // XPath

} // XMLTool
