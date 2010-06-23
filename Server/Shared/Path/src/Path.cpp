/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Path.h"

#include "PathNode.h"

#include "StringUtility.h"

#include <algorithm>
#include <iterator>

namespace Path {

namespace Impl {

NameNode* fillNode( NameNode* node, const vector<MC2String>& nodeNames ) {
   NameNode* lastNode = node;
   if ( nodeNames.size() > 1 ) {
      for ( uint32 i = 1; i < nodeNames.size(); ++i ) {
         NameNode* node = new NameNode( nodeNames[ i ] );
         lastNode->addNode( node );
         lastNode = node;
      }
   }
   return lastNode;
}

pair<NameNode*, uint32> findDepth( NameNode* lastNode,
                                   const vector<MC2String>& names,
                                   uint32 depth ) {
   if ( names.size() == depth ) {
      return make_pair( lastNode, depth - 1 );
   }

   NodeList::iterator it = lastNode->begin();
   NodeList::iterator itEnd = lastNode->end();
   for ( ; it != itEnd; ++it ) {
      if ( (*it)->getName() == names[ depth ] ) {
         return findDepth( (*it), names, depth + 1 );
      }
   }

   return pair<NameNode*, uint32>( lastNode, depth );
}

pair<NameNode*, uint32> findDepth( NodeList& list,
                                   const vector<MC2String>& names ) {
   // for all root nodes, search for first name
   for ( NodeList::iterator it = list.begin(); it != list.end(); ++it ) {
      if ( (*it)->getName() == names[ 0 ] ) {
         return findDepth( (*it), names, 1 );
      }
   }
   return pair<NameNode*, uint32>( 0, 0 );
}
/// prints name node tree recursively
void printNode( ostream& ostr, const NameNode& node, uint32 depth ) {
   const uint32 depthStep = 2;
   if ( node.empty() ) {
      // print attrib styl if this node is an attribute
      if ( node.isAttributeNode() ) {
         ostr << "<" << node.getName() << "=";
         if ( node.isAttributeValueNode() ) {
            ostr << "\"" << node.getAttributeValue() << "\" />" << endl;
         } else {
            ostr << "? />"  << endl;
         }
      } else {
         // else this node is a leaf
         ostr << "<" << node.getName()
              << ( node.isMultiNode() ? "*" : "" )
              << "/>" << endl;
      }
      // nothing more to do here.
      return;
   }
   ostr << "<" << node.getName()
        << ( node.isMultiNode() ? "*" : "" )
        << ">" << endl;

   NodeList::const_iterator subNode = node.begin();
   NodeList::const_iterator subNodeEnd = node.end();
   for ( ; subNode != subNodeEnd; ++subNode ) {
      fill_n( ostream_iterator<char>( ostr, "" ), depth, ' ' );

      printNode( ostr, *(*subNode), depth + depthStep );
   }
   fill_n( ostream_iterator<char>( ostr, "" ), depth - depthStep, ' ' );
   ostr << "</" << node.getName()
        << ( node.isMultiNode() ? "*" : "" )
        << ">" << endl;

}

} // Impl

} // Path
