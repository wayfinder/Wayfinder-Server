/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PATH_NODE_H
#define PATH_NODE_H

#include "MC2String.h"
#include "DeleteHelpers.h"

#include <list>

namespace Path {

class NameNode;

typedef std::list<NameNode*> NodeList;

/**
 * A node in a name tree.
 * Holds its name and all of its children
 */
class NameNode {
public:

   typedef NodeList::iterator iterator;
   typedef NodeList::const_iterator const_iterator;
   typedef NodeList::value_type value_type;

   /// @param name the name of the node
   explicit NameNode( const MC2String& name ):
      m_name( name ),
      m_attribNode( false ),
      m_isAttrValueNode( false ),
      m_multiNode( false ) {
      // empty names are strange
      // and so are single '*' names
      if ( name.empty() || name.size() == 1 ) {
         return;
      }

      // determine if this is a multinode
      m_multiNode = ( m_name[ name.size() - 1 ] == '*' );
      if ( m_multiNode ) {
         // this is a node we should check more than once.
         // erase the '*' so we can do normal name compare
         m_name.erase( m_name.size() - 1 );
      }
      // determine if this is an attribute node
      // '@' indicates attribute
      m_attribNode = ( m_name[ 0 ] == '@' );
      if ( m_attribNode ) {
         // cut out the attrib part of '@attrib'
         m_name.erase( 0, 1 );
      }
      // determine if the attrib has a value speciefied
      MC2String::size_type pos = m_name.find_first_of( "=" );
      if ( m_attribNode && pos != MC2String::npos ) {
         m_value = m_name.substr( pos + 1 );
         m_name.erase( pos, MC2String::npos );
         m_isAttrValueNode = true;
      }
   }

   /// @return the name of this node
   const MC2String& getName() const { return m_name; }

   /// @return the attribute value name
   const MC2String& getAttributeValue() const { return m_value; }

   /// @return start iterator for children
   iterator begin() { return m_nodes.begin(); }
   /// @return start iterator for children
   const_iterator begin() const { return m_nodes.begin(); }
   /// @return end iterator for children
   iterator end() { return m_nodes.end(); }
   /// @return end iterator for children
   const_iterator end() const { return m_nodes.end(); }
   /// @return true if there is no child nodes in this node = leaf node.
   bool empty() const { return m_nodes.empty(); }

   /// @return true if this node should be used more than once
   bool isMultiNode() const { return m_multiNode; }

   /// @return true if this node should look for attribute name
   bool isAttributeNode() const { return m_attribNode; }

   /// @return true if this node has a specific value for the attribute name
   bool isAttributeValueNode() const { return m_isAttrValueNode; }

   /**
    * adds a node to the node list
    * @param node the node to add, the node will be owned by this node.
    */
   void addNode( NameNode* node ) { m_nodes.push_back( node ); }

   /// copy the list
   void copyList( const NodeList& list ) {
      m_nodes.insert( m_nodes.end(), list.begin(), list.end() );
   }
private:

   typedef STLUtility::AutoContainer<NodeList > AutoNodeList;

   AutoNodeList m_nodes; //< child nodes

   MC2String m_name; //< name of this node
   MC2String m_value; //< the value if a value is speciefied
   bool m_attribNode; //< true if this node is an attribute
   bool m_isAttrValueNode; //< true if this node has a value specified
   bool m_multiNode; //< whether this node should match more than one
};

} // Path

#endif // PATH_NODE_H
