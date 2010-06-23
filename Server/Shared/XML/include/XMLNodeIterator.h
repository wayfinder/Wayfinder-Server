/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLTOOL_NODEITERATOR_H
#define XMLTOOL_NODEITERATOR_H

#include <dom/DOM.hpp>
#include <iterator>

#if (XERCES_VERSION_MAJOR == 2 && XERCES_VERSION_MINOR >= 4) || (XERCES_VERSION_MAJOR > 2)
using namespace xercesc;
#endif

namespace XMLTool {

/**
 * Simple node iterator for specific node types
 * For an iterator that iterates through nodes of element type and
 * returns const DOMNode pointer: 
 * \code
 * NodeIterator<DOMNode::ELEMENT_NODE, const DOMNode> it( node );
 * NodeIterator<DOMNode::ELEMENT_NODE, const DOMNode> end( NULL );
 * for ( ; it != end; ++it ) {
 *    cerr << it->getNodeName() << endl;
 * }
 * \endcode
 * This is cleaner than the xerces implementation with filter and 
 * dom iterator.
 * This can also be used with STL algorithms. For example:
 * \code
 * void parseTree( const DOMNode* node, int indentLevel ) {
 *    std::fill_n( std::ostream_iterator<char>( cout ), indentLevel, ' ');
 *    cout << "Name: " << node->getNodeName() << endl;
 *    std::for_each( ElementConstIterator( node->getFirstChild() ), 
 *                   ElementConstIterator( NULL ),
 *                   std::bind2nd( ptr_fun( &parseTree ), indentLevel + 3 ) );
 * }
 * \endcode
 * This will print the entire node tree indented.
 */
template <DOMNode::NodeType Type, typename PtrType>
class NodeIterator: public std::iterator<std::forward_iterator_tag, PtrType*> {
public:
   typedef PtrType* Ptr;

   NodeIterator():m_node( NULL ) { }

   explicit NodeIterator( Ptr node ):
      m_node( node ) {
      while ( m_node && m_node->getNodeType() != Type ) {
         m_node = m_node->getNextSibling();
      }
   }

   /**
    * Go to next element.
    * @return current value.
    */
   NodeIterator<Type, PtrType>& operator ++() {
      nextElement();
      return *this;
   }

   /**
    * Go to next element.
    * @return old value.
    */
   NodeIterator<Type, PtrType> operator ++(int) {
      NodeIterator<Type, PtrType> oldValue( *this );
      nextElement();
      return oldValue;
   }

   /// @return current node
   Ptr operator ->() const {
      return m_node;
   }
   /// @return current node
   Ptr operator *() const {
      return m_node;
   }

   bool operator == ( const NodeIterator<Type, PtrType>& other ) const {
      return other.m_node == m_node;
   }

   bool operator != ( const NodeIterator<Type, PtrType>& other ) const {
      return ! ( *this == other );
   }

private:
   void nextElement() {
      do {
         m_node = m_node->getNextSibling();
      } while ( m_node && m_node->getNodeType() != Type );
   }

   Ptr m_node;
};

/// For iterating through elements
typedef NodeIterator<DOMNode::ELEMENT_NODE, DOMNode> ElementIterator;
/// For iterating through const elements
typedef NodeIterator<DOMNode::ELEMENT_NODE, const DOMNode> ElementConstIterator;


}

#endif // XMLTOOL_NODEITERATOR_H
