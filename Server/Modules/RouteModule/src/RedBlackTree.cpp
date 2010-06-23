/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RedBlackTree.h"
#ifdef OLD_RB_TREE_IN_RM

RedBlackTree::RedBlackTree()
{
   nullSentinel = new RedBlackNode();
   initNullSentinel();
   
   tree = nullSentinel;             // In the beginning, there where NULL.
}


RedBlackTree::~RedBlackTree()
{
   reset();
   delete nullSentinel;
}


RoutingNode *RedBlackTree::dequeue()
{
   RedBlackNode* minNode = seekMin();

   initNullSentinel();
   
   rbDelete(minNode);
   RoutingNode* minimumNode = minNode->routingNode;
   delete minNode;
   return minimumNode;
}


void RedBlackTree::dump()
{
   dumpTree(tree);
}


void RedBlackTree::dumpTree(RedBlackNode* node)
{
   if (node != nullSentinel) {
      node->dump();
      dumpTree(node->leftChild);
      dumpTree(node->rightChild);
   }
}

void RedBlackTree::enqueue(RoutingNode *node)
{
   initNullSentinel();

   RedBlackNode *y;

   RedBlackNode* x = new RedBlackNode(node);
   (*x).setNullSentinel(nullSentinel);

   treeInsert(x);
   x->color = RED;

   while (x != tree && x->parent->color == RED)
      if (x->parent == x->parent->parent->leftChild) {
         y = x->parent->parent->rightChild;
         if (y->color == RED) {
            x->parent->color = BLACK;
            y->color = BLACK;
            x->parent->parent->color = RED;
            x = x->parent->parent;
         }
         else {
            if (x == x->parent->rightChild) {
               x = x->parent;
               leftRotate(x);
            }
            x->parent->color = BLACK;
            x->parent->parent->color = RED;
            rightRotate(x->parent->parent);
         }
      }
      else {
         y = x->parent->parent->leftChild;
         if (y->color == RED) {
            x->parent->color = BLACK;
            y->color = BLACK;
            x->parent->parent->color = RED;
            x = x->parent->parent;
         }
         else {
            if (x == x->parent->leftChild) {
               x = x->parent;
               rightRotate(x);
            }
            x->parent->color = BLACK;
            x->parent->parent->color = RED;
            leftRotate(x->parent->parent);
         }
      } // end of if-case

   tree->color = BLACK;
} // enqueue


void RedBlackTree::rbFixup(RedBlackNode* x)
{
   RedBlackNode* w;
   while (x != tree && x->color == BLACK)
      if (x == x->parent->leftChild) {
         w = x->parent->rightChild;
         if (w->color == RED) {
            w->color = BLACK;
            x->parent->color = RED;
            leftRotate(x->parent);
            w = x->parent->rightChild;
         }
         if (w->leftChild->color == BLACK && w->rightChild->color == BLACK) {
            w->color = RED;
            x = x->parent;
         }
         else {
            if (w->rightChild->color == BLACK) {
               w->leftChild->color = BLACK;
               w->color = RED;
               rightRotate(w);
               w = x->parent->rightChild;
            }
            w->color = x->parent->color;
            x->parent->color = BLACK;
            w->rightChild->color = BLACK;
            leftRotate(x->parent);
            x = tree;
         }
      }
      else {
         w = x->parent->leftChild;
         if (w->color == RED) {
            w->color = BLACK;
            x->parent->color = RED;
            rightRotate(x->parent);
            w = x->parent->leftChild;
         }
         if (w->rightChild->color == BLACK && w->leftChild->color == BLACK) {
            w->color = RED;
            x = x->parent;
         }
         else {
            if (w->leftChild->color == BLACK) {
               w->rightChild->color = BLACK;
               w->color = RED;
               leftRotate(w);
               w = x->parent->leftChild;
            }
            w->color = x->parent->color;
            x->parent->color = BLACK;
            w->leftChild->color = BLACK;
            rightRotate(x->parent);
            x = tree;
         }
      }
   x->color = BLACK;
} // rbFixup


void RedBlackTree::treeInsert(RedBlackNode* z)
{
      // See Introduction to algorithms, p. 251
   RedBlackNode* y = nullSentinel;
   RedBlackNode* x = tree;
   while (x != nullSentinel) {
      y = x;
      if (z->key < x->key)
         x = x->leftChild;
      else
         x = x->rightChild;
   }
   z->parent = y;
   if (y == nullSentinel)
      tree = z;
   else
      if (z->key < y->key)
         y->leftChild = z;
      else
         y->rightChild = z;
} // treeInsert


void RedBlackTree::leftRotate(RedBlackNode* x)
{
   RedBlackNode* y = x->rightChild;
   x->rightChild = y->leftChild;
   if (y->leftChild != nullSentinel)
      y->leftChild->parent = x;
   y->parent = x->parent;

   if (x->parent == nullSentinel)
      tree = y;
   else
      if (x == x->parent->leftChild)
         x->parent->leftChild = y;
      else
         x->parent->rightChild = y;

   y->leftChild = x;
   x->parent = y;
} // leftRotate


void RedBlackTree::rightRotate(RedBlackNode* x)
{
   RedBlackNode* y = x->leftChild;
   x->leftChild = y->rightChild;
   if (y->rightChild != nullSentinel)
      y->rightChild->parent = x;
   y->parent = x->parent;

   if (x->parent == nullSentinel)
      tree = y;
   else
      if (x == x->parent->rightChild)
         x->parent->rightChild = y;
      else
         x->parent->leftChild = y;

   y->rightChild = x;
   x->parent = y;
} // rightRotate


void RedBlackTree::rbDelete(RedBlackNode* y)
{
      // y == aNode in RedBlackTree.h
   RedBlackNode *x;

/*
   // These rows are supposed to be removed later. This case ought not occur
   // and has never done so.
   if (y->leftChild != nullSentinel) {
      x = y->leftChild;
      mc2log << error
      << "ERROR: Serious bug! RedBlackTree is dequeuing incorrectly!"
           << endl;
      exit(1);
   }
   else
*/
      x = y->rightChild;

   x->parent = y->parent;

   if (y->parent == nullSentinel)
      tree = x;
   else
      if (y == y->parent->leftChild)
         y->parent->leftChild = x;
      else
         y->parent->rightChild = x;

   if (y->color == BLACK)
      rbFixup(x);

} // rbDelete

#endif
