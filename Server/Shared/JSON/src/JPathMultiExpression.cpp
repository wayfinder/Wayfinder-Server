/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "JPathMultiExpression.h"

#include "json_spirit/json_spirit.h"

#include "Path.h"

#include <set>

namespace MC2JSON {

namespace JPath {

void doCallback( MultiExpression::NodeEvaluator& eval,
                 const NodeType& node,
                 const MC2String& name ) {
   try {
      eval( node );
   } catch ( const EvaluateException& e ) {
      mc2dbg << "[JSON::JPath::MultiExpression] In node \""
             << name << "\": " << e.what() << endl;
   }
}

MultiExpression::MultiExpression( const Description& nodes ):
   m_rootNode( new Path::NameNode( "evaluation tree" ) ) {
   // build tree
   Path::createNodeTree( *m_rootNode, m_functions, nodes, "" );

}

MultiExpression::~MultiExpression() {
   STLUtility::deleteAllSecond( m_functions );
}

void MultiExpression::
evaluateObject( const NodeType& root, const Path::NameNode* node ) const {

   // Holds nodes to skip when running evaluate,
   // i.e if they are single hit nodes ( without '*')
   set<const Path::NameNode*> skip;

   const json_spirit::Object& object = root.get_obj();

   for ( json_spirit::Object::const_iterator objIt = object.begin();
         objIt != object.end(); ++objIt ) {
      const MC2String& nodeName = (*objIt).name_;
      const NodeType& nextNode = (*objIt).value_;
      for ( Path::NodeList::const_iterator evalNodeIt = node->begin();
            evalNodeIt != node->end(); ++evalNodeIt ) {
         Path::NodeList::value_type evalNode = *evalNodeIt;

         // search skip nodes and then compare name
         if ( skip.find( evalNode ) != skip.end() ) {
            // found a name node which we should skip
            // or the name did not match
            continue;
         }

         if ( nodeName != evalNode->getName() ) {
            continue;
         }

         // if node is empty then we have a leaf,
         // which means we should call the corresponding functor
         if ( evalNode->empty() ) {
            // call functor with this node, through evaluation of the next
            // value, since the value is in the next node
            evaluate( nextNode, evalNode );
         } else {
            // the evaluation node is not empty, which means we
            // have to go deaper in to the tree
            // but first see if there is a functor for this node and call it
            EvaluatorMap::const_iterator eval = m_functions.find( evalNode );
            if ( eval != m_functions.end() ) {
               // call functor with this node
               doCallback( const_cast<NodeEvaluator&>(*(*eval).second),
                           nextNode,
                           (*eval).first->getName() );
            }
            // now we can go deeper
            evaluate( nextNode, evalNode );
         }
      }
   }
}

void MultiExpression::evaluate( const NodeType& root,
                                const Path::NameNode* node ) const {

   // The algorithm is as follows:
   // TODO: Update this documentation to JSON.
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

   if ( root == root.null ) {
    // no need to parse empty data
   } else if ( root.type() == json_spirit::array_type ) {
      const json_spirit::Array& array = root.get_array();
      for ( json_spirit::Array::const_iterator it = array.begin();
            it != array.end(); ++it ) {
         evaluate( *it, node );
      }
   } else if ( root.type() == json_spirit::obj_type ) {
      evaluateObject( root, node );

// What does this do?
//   } else if ( root.type() == typeid ( boost::shared_ptr< boost::any > ) ) {
//      evaluate( *boost::any_cast< boost::shared_ptr< boost::any > >( root ),
//                node );
   } else {

      EvaluatorMap::const_iterator eval = m_functions.find( node );
      if ( eval == m_functions.end() ) {
         mc2log << warn
                << "[JPath::ME] Could not find function for leaf node: "
                << node->getName() << endl;
      } else {
         // call functor with this node
         doCallback( const_cast<NodeEvaluator&>(*(*eval).second),
                     root,
                     (*eval).first->getName() );
      }

   }

}

void MultiExpression::evaluate( const NodeType& root ) const {
   evaluate( root, m_rootNode.get() );
}


} // JPath
} // MC2JSON
