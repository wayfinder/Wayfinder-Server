/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REDBLACKNODE_H
#define REDBLACKNODE_H

#include "config.h"
#include "RoutingNode.h"


enum colorType { RED, BLACK };

/**
 *  Describes a node in the Red-Black Tree ADT.
 *  A special node is nullSentinel that replaces NULL for each tree.
 *
 */
class RedBlackNode {
   friend class RedBlackTree;
   
  public:
   
/**
 * Constructor for the RedBlackNode.
 * @param node The node to be inserted.
 */
   RedBlackNode(RoutingMap* theMap, RoutingNode *node);

/**
 * Another constructor for the node.
 * Only used to construct nullSentinel.
 */
   RedBlackNode();

/**
 * Sets the parent and children pointers of the current node
 * to nullSentinel. 
 * @param nullSentinel The nullSentinel in the tree.
 */
   inline void setNullSentinel(RedBlackNode* nullSentinel);

/**
 * Dumps the itemID of the node to cout. Both hex and dec.
 */
   inline void dump();
   
  private:

/**
 * The color of the node.
 */
   colorType color;
   
/**
 * The key of the node.
 */
   uint32 key;

/**
 * The left child of the node.
 */
   RedBlackNode *leftChild;

/**
 * The right child of the node.
 */
   RedBlackNode *rightChild;
   
/**
 * The parent of the node.
 */
   RedBlackNode* parent;
   
/**
 * The node in the map.
 */
   RoutingNode* routingNode;

}; // RedBlackNode

/////////////////////////////////////////////////////////////
// Inline functions
/////////////////////////////////////////////////////////////

void RedBlackNode::setNullSentinel(RedBlackNode* nullSentinel)
{
   leftChild = rightChild = parent = nullSentinel;
}

void RedBlackNode::dump()
{
   if (routingNode != NULL)
      cout << "ID " << hex << routingNode->getItemID() << dec << "("
           << REMOVE_UINT32_MSB( routingNode->getItemID() ) << ")" << endl;
}

#endif
