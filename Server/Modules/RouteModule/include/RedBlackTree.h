/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

#include "config.h"
#include "RoutingNode.h"

class RoutingMap;

#undef  OLD_RB_TREE_IN_RM
#ifdef  OLD_RB_TREE_IN_RM
#include "RedBlackNode.h"

/**
 * Describes the Red-Black Tree ADT. It is a heap type used by the Dijkstra
 * algorithm in RouteModule.
 *
 */
class RedBlackTree
{
  public:
   
/**
 * Constructor for the RedBlackTree.
 */
   RedBlackTree(const RoutingMap* theMap);
   
/**
 * Destructor for the RedBlackTree.
 */
   ~RedBlackTree();
   
/**
 * Reset function for the Red-Black Tree.
 * Supposed to be ran before the tree is used.
 * Deletes what is left of the tree.
 */
   inline void reset();
   
/**
 * Returns true if the tree is empty, false otherwise.
 * @return True if tree is empty.
 */
   inline bool isEmpty();
   
/**
 * Removes a RoutingNode from the tree ("deleteMin").
 * @return A routingNode with the minimum cost.
 */
   RoutingNode *dequeue();

/**
 * Puts a RoutingNode into the tree.
 * @param node the node to be enqueued.
 */
   void enqueue(RoutingNode *node);

/**
 * Dumps the entire RedBlack Tree's nodeIDs to cout.
 */
   void dump();

/**
 * Dumps the node and the subtree of node to cout.
 */
   void dumpTree(RedBlackNode* node);

  private:

/**
 * Fixes the tree after a deletion according to the ADT-algorithm.
 * @param node The node to fix the tree around.
 */
   void rbFixup(RedBlackNode* aNode);

/**
 * Inserts a node into the tree without caring about the RedBlack structure.
 * @param node The node to be inserted.
 */
   void treeInsert(RedBlackNode* graphNode);

/**
 * Rotates the tree leftwards around the input node.
 * @param node The node to rotate around.
 */
   void leftRotate(RedBlackNode* graphNode);
  
/**
 * Rotates the tree rightwards around the input node.
 * @param Node the node to rotate around.
 */
   void rightRotate(RedBlackNode* graphNode);

/**
 * Deletes a node.
 * @param Node the node to be deleted.
 */
   void rbDelete(RedBlackNode* aNode);

/**
 * Returns the minimum node of the RedBlack Tree.
 * @return minNode The minimum node.
 */
   inline RedBlackNode* seekMin();

/**
 * Initializes the nullSentinel.
 * Has to be done before enqueue and dequeue.
 */
   inline void initNullSentinel();

//////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////
   
/**
 * Pointer to a node containing the minimum key.
 */
   RedBlackNode* tree;

/**
 * Describes the node that replaces NULL.
 */
   RedBlackNode* nullSentinel;

}; // RedBlackTree

/////////////////////////////////////////////////////////////////
// Inline functions
/////////////////////////////////////////////////////////////////

void RedBlackTree::reset()
{
   while (tree != nullSentinel)
      dequeue();
}

bool RedBlackTree::isEmpty()
{
   return tree == nullSentinel;
}

RedBlackNode* RedBlackTree::seekMin()
{
   RedBlackNode *minNode = tree;

   if (minNode != nullSentinel)
      while (minNode->leftChild != nullSentinel)
         minNode = minNode->leftChild;

   return minNode;
}

void RedBlackTree::initNullSentinel()
{
   nullSentinel->color = BLACK;
   nullSentinel->leftChild = nullSentinel;
   nullSentinel->rightChild = nullSentinel;
   nullSentinel->parent = nullSentinel;
}

#else

#include<map>
#include<queue>

/**
 * Describes the Red-Black Tree ADT. It is a heap type used by the Dijkstra
 * algorithm in RouteModule.
 *
 * New version. Uses stl multimap.
 *
 */

class RedBlackTree /* : private multimap<uint32, RoutingNode*> */
: private priority_queue<pair<uint32, RoutingNode* > >
{
public:

   /**
    *   For use as a bucket in BucketHeap.
    */
   RedBlackTree(const RoutingMap* theMap, int size = 0);
   
   /**
    * Reset function for the Red-Black Tree.
    * Supposed to be ran before the tree is used.
    * Deletes what is left of the tree.
    */
   inline void reset();
   
   /**
    * Returns true if the tree is empty, false otherwise.
    * @return True if tree is empty.
    */
   inline bool isEmpty() const;
   
   /**
    *   Removes a RoutingNode from the tree ("deleteMin").
    *   @return A routingNode with the minimum cost.
    */
   inline RoutingNode* dequeue();

   /**
    *   Does nothing. For compatibility with BucketHeap.
    */
   inline void updateStartIndex(uint32);
   
   /**
    *   Puts a RoutingNode into the tree.
    *   @param node the node to be enqueued.
    */
   inline void enqueue(RoutingNode *node);

   /**
    *   Does nothing.
    */
   inline void dump();
  private:
   
   /**
    *   Pointer to the map. Needed for costs.
    */
   const RoutingMap* m_map;
};

inline
RedBlackTree::RedBlackTree(const RoutingMap* theMap,
                           int size) : /* multimap<uint32, RoutingNode*>() */
      priority_queue<pair<uint32, RoutingNode*> >()
{
   m_map = theMap;
}

inline void
RedBlackTree::reset()
{
   //clear();
   while ( ! empty() ) {
      pop();
   }
}

inline bool
RedBlackTree::isEmpty() const
{
   return empty();
}

inline RoutingNode*
RedBlackTree::dequeue()
{
   // For use as a bucket.
/*     if ( empty() ) */
/*        return NULL; */
/*     iterator it = begin(); */
/*     RoutingNode* node = it->second; */
/*     erase(it); */
/*     return node; */
   RoutingNode* topNode = top().second;
   pop();
   return topNode;
}

inline void
RedBlackTree::enqueue(RoutingNode* node)
{
   //insert(pair<uint32, RoutingNode*>(node->getEstCost(), node));
   // Priority queue dequeues the most expensive first.
   push(make_pair(MAX_UINT32-node->getEstCost(m_map), node));
}

inline void
RedBlackTree::dump()
{
}

inline void
RedBlackTree::updateStartIndex(uint32)
{

}

#endif
#endif
