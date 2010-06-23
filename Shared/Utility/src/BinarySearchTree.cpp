/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BinarySearchTree.h"
//#include <iostream.h>

BinarySearchTree::BinarySearchTree() {
   nullSentinel = new Sentinel;
   nullSentinel->color = false;
   nullSentinel->parent = NULL;
   root = nullSentinel;
   cardinal = 0;
}
 
BinarySearchTree::~BinarySearchTree() {
   if ( root != nullSentinel ) {
      delete root;
      delete nullSentinel;
   } else {
      delete root; // And nullSentinel in one delete
   }
}

void
BinarySearchTree::leftRotate(BinarySearchTreeNode *x) {
   BinarySearchTreeNode *y;
   y = x->right;
   x->right = y->left;
   if(y->left != nullSentinel) {
      y->left->parent = x;
   }

   y->parent = x->parent;
   
   if(x->parent == nullSentinel) {
      root = y;
   }
   else {
      if(x == x->parent->left) {
         x->parent->left = y;
      }
      else {
         x->parent->right = y;
      }
   }

   y->left = x;
   x->parent = y;
}

void
BinarySearchTree::rightRotate(BinarySearchTreeNode *x) {
   BinarySearchTreeNode *y;

   y = x->left;
   x->left = y->right;
   if(y->right != nullSentinel) {
      y->right->parent = x;
   }

   y->parent = x->parent;

   if(x->parent == nullSentinel) {
      root = y;
   }
   else {
      if(x == x->parent->right) {
         x->parent->right = y;
      }
      else {
         x->parent->left = y;
      }
   }
   y->right = x;
   x->parent = y;
}



void
BinarySearchTree::treeInsert(BinarySearchTreeNode *z) {
   BinarySearchTreeNode *y = nullSentinel;
   BinarySearchTreeNode *x = root;
   while(x != nullSentinel) {
      y = x;
      if(*z < *x) {
         x = x->left;
      }
      else {
         x = x->right;
      }
   }
   z->parent = y;
   if(y == nullSentinel) {
      root = z;
   }
   else {
      if(*z < *y) {
         y->left = z;
      }
      else {
         y->right = z;
      }
   }
}


void
BinarySearchTree::remove(BinarySearchTreeNode *z) {
   BinarySearchTreeNode *x;
   BinarySearchTreeNode *y;
   
   nullSentinel->color = false;
   nullSentinel->parent = NULL;
   
   if(z->left == nullSentinel || z->right == nullSentinel) {
      y = z;
   }
   else {
      y = z->suc();
   }

   // z->suc() might return NULL, and there
   // is nothing we can do after that.
   if ( y == NULL ) {
      return;
   }

   if(y->left != nullSentinel) {
      x = y->left;
   }
   else {
      x = y->right;
   }

   x->parent = y->parent;

   if(y->parent == nullSentinel) {
      root = x;
   }
   else {
      if(y == y->parent->left) {
         y->parent->left = x;
      }
      else {
         y->parent->right = x;
      }
   }

   if(y != z) {
      y->parent = z->parent;
      y->right = z->right;
      y->left = z->left;
      bool color = y->color;
      y->color = z->color;
      z->color = color;
      y->left->parent = y;
      y->right->parent = y;
      
      if(z == root) {
         root = y;
      }
      else {
         if(z->parent->left == z) {
            z->parent->left = y;
         }
         else {
            z->parent->right = y;
         }
      }
   }
   if(z->color == false) {
      deleteFixup(x);
   }

   z->parent = NULL;
   z->left = NULL;
   z->right = NULL;
   
   cardinal--;
   nullSentinel->parent = NULL;
}


void
BinarySearchTree::deleteFixup(BinarySearchTreeNode *x) {
   BinarySearchTreeNode *w;
   while(x != root && x->color == false) {
      if(x == x->parent->left) {
         w = x->parent->right;
         if(w->color == true) {
            w->color = false;
            x->parent->color = true;
            leftRotate(x->parent);
            w = x->parent->right;
         }
         if(w->left->color == false && w->right->color == false) {
            w->color = true;
            x = x->parent;
         }
         else {
            if(w->right->color == false) {
               w->left->color = false;
               w->color = true;
               rightRotate(w);
               w = x->parent->right;
            }
            w->color = x->parent->color;
            x->parent->color = false;
            w->right->color = false;
            leftRotate(x->parent);
            x = root;
         }
      }
      else {
         w = x->parent->left;
         if(w->color == true) {
            w->color = false;
            x->parent->color = true;
            rightRotate(x->parent);
            w = x->parent->left;
         }
         if(w->left->color == false && w->right->color == false) {
            w->color = true;
            x = x->parent;
         }
         else {
            if(w->left->color == false) {
               w->right->color = false;
               w->color = true;
               leftRotate(w);
               w = x->parent->left;
            }
            w->color = x->parent->color;
            x->parent->color = false;
            w->left->color = false;
            rightRotate(x->parent);
            x = root;
         }
      }
   }
   x->color = false;
}

void
BinarySearchTree::add(BinarySearchTreeNode *x) {
   x->right = nullSentinel;
   x->left = nullSentinel;
   x->color = true;
   nullSentinel->color = false;
   
   BinarySearchTreeNode *y;
   
   treeInsert(x);
   while(x != root && x->parent->color == true) {
      if(x->parent == x->parent->parent->left) {
         y = x->parent->parent->right;
         if(y->color == true) {
            x->parent->color = false;
            y->color = false;
            x->parent->parent->color = true;
            x = x->parent->parent;
         }
         else {
            if(x == x->parent->right) {
               x = x->parent;
               leftRotate(x);
            }
            x->parent->color = false;
            x->parent->parent->color = true;
            rightRotate(x->parent->parent);
         }
      }
      else {
         y = x->parent->parent->left;
         if(y->color == true) {
            x->parent->color = false;
            y->color = false;
            x->parent->parent->color = true;
            x = x->parent->parent;
         }
         else {
            if(x == x->parent->left) {
               x = x->parent;
               rightRotate(x);
            }
            x->parent->color = false;
            x->parent->parent->color = true;
            leftRotate(x->parent->parent);
         }
      }
   }
   root->color = false;
   cardinal++;
   nullSentinel->parent = NULL;
}
      
BinarySearchTreeNode *
BinarySearchTree::getMin() const {
   BinarySearchTreeNode *node = root;
   if(node != nullSentinel) {
      while(node->left != nullSentinel) {
         node = node->left;
      }
      return node;
   }
   else {
      return NULL;
   }
}

BinarySearchTreeNode *
BinarySearchTree::getMax() const {
   BinarySearchTreeNode *node = root;
   if(node != nullSentinel) {
      while(node->right != nullSentinel) {
         node = node->right;
      }
      return node;
   }
   else {
      return NULL;
   }
}

BinarySearchTreeNode *
BinarySearchTree::getRoot() const {
   if(root != nullSentinel)
      return root;
   else
      return NULL;
}

uint32
BinarySearchTree::getCardinal() const {
   return cardinal;
}

BinarySearchTreeNode *
BinarySearchTree::equal(BinarySearchTreeNode *node) const {
   BinarySearchTreeNode *tempNode = NULL;
   if(root != nullSentinel) {
      tempNode = root->search(node);
      if(!(*tempNode == *node))
         tempNode = NULL;
   }
   return tempNode;
}

BinarySearchTreeNode *
BinarySearchTree::greaterOrEqual(BinarySearchTreeNode *node) const {
   BinarySearchTreeNode *tempNode = NULL;
   if(root != nullSentinel) {
      tempNode = root->search(node);
      if(*tempNode < *node)
         tempNode = NULL;
   }
   return tempNode;
}

BinarySearchTreeNode *
BinarySearchTree::lessOrEqual(BinarySearchTreeNode *node) const {
   BinarySearchTreeNode *tempNode = NULL;
   if(root != nullSentinel) {
      tempNode = root->search(node);
      if(*tempNode > *node)
         tempNode = tempNode->pred();
      if(tempNode == nullSentinel)
         tempNode = NULL;
   }
   return tempNode;
}

BinarySearchTreeNode::BinarySearchTreeNode() {
   parent = NULL;
   left = NULL;
   right = NULL;
   color = false;
}

BinarySearchTreeNode::~BinarySearchTreeNode() {
   DEBUG8( cerr << "~BinarySearchTreeNode() start" << endl );
   if( (left != NULL) && (left->parent != NULL) ) {
      delete left;
   }
   if( ( right != NULL) && (right->parent != NULL) ) {
      delete right;
   }
   DEBUG8( cerr << "~BinarySearchTreeNode() end" << endl );
}

BinarySearchTreeNode *
BinarySearchTreeNode::suc() const {
   if(right->parent != NULL) {
      BinarySearchTreeNode *node = right;
      while(node->left->parent != NULL)
         node = node->left;
      if(node->parent != NULL)
         return node;
      else
         return NULL;
   }
   BinarySearchTreeNode *y = parent;
   const BinarySearchTreeNode *x = this;
   while(y->parent != NULL && x == y->right) {
      x = y;
      y = x->parent;
   }
   if(y->parent != NULL)
      return y;
   else
      return NULL;
}

BinarySearchTreeNode *
BinarySearchTreeNode::pred() const {
   if(left->parent != NULL) {
      BinarySearchTreeNode *node = left;
      while(node->right->parent != NULL)
         node = node->right;
      if(node->parent != NULL)
         return node;
      else
         return NULL;
   }
   BinarySearchTreeNode *node = parent;
   const BinarySearchTreeNode *tempNode = this;
   while(node->parent != NULL && tempNode == node->left) {
      tempNode = node;
      node = node->parent;
   }
   if(node->parent != NULL)
      return node;
   else
      return NULL;
}

BinarySearchTreeNode *
BinarySearchTreeNode::getLeft() const {
   if(left->parent == 0)
      return NULL;
   else
      return left;
}

BinarySearchTreeNode *
BinarySearchTreeNode::getRight() const {
   if(right->parent == 0)
      return NULL;
   else
      return right;
}

BinarySearchTreeNode *
BinarySearchTreeNode::search(BinarySearchTreeNode *node) const {
   if( *this == *node) {
      BinarySearchTreeNode* res = const_cast< BinarySearchTreeNode* >( this );
      return res;
   }

   if(*this > *node) {
      if(left->parent != NULL) {
         return left->search(node);
      }
      else {
         BinarySearchTreeNode* res = const_cast<BinarySearchTreeNode*>( this );
         return res;
      }
   }
   else {
      if(right->parent != NULL) {
         return right->search(node);
      }
      else {
         BinarySearchTreeNode* res = const_cast<BinarySearchTreeNode*>( this );
         return res;
      }
   }
}
