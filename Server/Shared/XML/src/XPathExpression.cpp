/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XPathExpression.h"

#include "StringUtility.h"
#include "DeleteHelpers.h"
#include "XMLTool.h"

namespace XMLTool {
namespace XPath {

Expression::Expression( const MC2String& expression,
                        const MC2String& nspace ):
   m_expression( expression ),
   m_root( false ),
   m_attribute( false ),
   m_nbrHits( -1 ),
   m_backStr( XMLUtility::transcodetoucs( ".." ) ) {
   // determine if the expression is a root expression
   // by checking first character for '/'
   size_t pos = expression.find_first_not_of(' ');
   if ( pos != MC2String::npos ) {
      m_root = expression[ pos ] == '/';
   }
   // parse to MC2Strings and then make them XMLCh* string
   vector<MC2String> nodeNames;
   StringUtility::tokenListToVector( nodeNames,
                                     expression, '/' );
   // append namespace to each string
   if ( ! nspace.empty() ) {
      MC2String newNspace = nspace + ":";
      for ( uint32 nodeIt = 0; nodeIt < nodeNames.size(); ++nodeIt ) {
         // add namespace to all nodes except for attributes
         if ( ! nodeNames[ nodeIt ].empty() && 
              nodeNames[ nodeIt ][ 0 ] != '@' ) {
            nodeNames[ nodeIt ].insert( 0, newNspace );
         }
      }
   }

   if ( !nodeNames.empty() ) {
      // find multiplier
      size_t startPos = nodeNames.back().find_last_of('*');
      // if multiplier found then we take all node we can find
      if ( startPos != MC2String::npos ) {
         // strip the last * from the string
         nodeNames.back().erase( nodeNames.back().size() - 1 );
      } else {
         // only one hit is enough
         m_nbrHits = 1;
      }

      // determine if leaf node is an attribute
      // '@' indicates attribute
      m_attribute = ( nodeNames.back()[ 0 ] == '@' );
      if ( m_attribute ) {
         nodeNames.back().erase( 0, 1 );
      }

      // transcode to XMLCh, which is faster once we compare strings
      // in the evaluation

      m_nodeNames.resize( nodeNames.size() );
      for ( uint32 i = 0; i < m_nodeNames.size(); ++i ) {
         m_nodeNames[ i ] =  XMLUtility::transcodetoucs( nodeNames[ i ] );
      }
      // recreate original expression, if needed.
      if ( ! nspace.empty() ) {
         if ( m_root ) {
            m_expression = "/";
         } else {
            m_expression.clear();
         }

         // skip loop if only one item in names
         if ( nodeNames.size() > 1 ) {
            // add all names except the last one
            for ( uint32 i = 0; i < nodeNames.size() - 1; ++i ) {
               m_expression += nodeNames[ i ] + "/";
            }
         }

         // add the last expression
         if ( m_attribute ) {
            m_expression += "@";
         }

         m_expression += nodeNames.back();

         if ( m_nbrHits != 1 ) {
            m_expression += "*";
         }
      }
   }

}

Expression::~Expression() {
   for ( uint32 i = 0; i < m_nodeNames.size(); ++i ) {
      delete [] m_nodeNames[ i ];
   }

   delete [] m_backStr;
}

MC2String Expression::getExpression() const {
   return m_expression;
}

void Expression::evaluate( const DOMNode* parent,
                           const DOMNode* nodeIn, 
                           vector<const DOMNode*>& nodes, 
                           uint32 depth, 
                           int32& nbrHits ) const {

   if ( m_attribute && depth + 1 == m_nodeNames.size() ) {
      // only search for attrib if we have a parent
      if ( parent ) {
         const DOMNode* attribNode = 
            findAttribConst( parent, 
                             XMLUtility::
                             transcodefrom( m_nodeNames[ depth ] ).c_str(),
                             false ); // ignore case 

         if ( attribNode != NULL ) {
            nodes.push_back( attribNode );
         }
      }
      
      return;
   }

   if ( nodeIn == NULL || depth == m_nodeNames.size() ) {
      return;
   }

   // go back one step if ".."
   if ( XMLString::compareString( m_nodeNames[ depth ], m_backStr ) == 0 ) {
      evaluate( parent ? parent->getParentNode() : NULL,
                parent, nodes, depth + 1, nbrHits );
      return;
   }

   for ( const DOMNode* node = nodeIn;
         node != NULL;
         node = node->getNextSibling() ) {

      mc2dbg8 << node->getNodeName() << endl;

      // only interested in element nodes and
      // the nodes that matches name in the correct depth
      if ( node->getNodeType() != DOMNode::ELEMENT_NODE ||
           XMLString::compareIString( node->getNodeName(),
                                      m_nodeNames[ depth ] ) != 0 ) {
         continue;
      }

      // final depth? then add this node
      // and try to find some more hits
      if ( depth == m_nodeNames.size() - 1 ) {
           
         nodes.push_back( node );
         if ( nbrHits >= 0 ) {
            nbrHits--;
         }
         // we are done once we got all hits
         if ( nbrHits == 0 ) {
            return;
         }

      } else {
         // else go deeper in this node
         evaluate( node, node->getFirstChild(),
                   nodes, depth + 1, nbrHits );
      }
   }
   

      
}

vector<const DOMNode*> Expression::evaluate( const DOMNode* node ) const {
   vector<const DOMNode*> nodes;
   if ( m_root ) {
      // get first root node
      // not yet implemented
   }

   int32 nbrHits = m_nbrHits;
   evaluate( node->getParentNode(), node, nodes, 0, nbrHits );

   return nodes;
}

} // XPath

} // XMLTool
