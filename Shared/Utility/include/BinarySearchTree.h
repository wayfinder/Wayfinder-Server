/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BINARYSEARCHTREE_H
#define BINARYSEARCHTREE_H

#include "config.h"
#include "NotCopyable.h"

/**
  *   Node in a binary search tree.
  *
  *   @see BinarySearchTree
  */
class BinarySearchTreeNode: private NotCopyable {
   protected:
      /**
       *    Default constructor. Declared protected to avoid usage from
       *    outside.
       */
      BinarySearchTreeNode();

   public:
      /**
       *    Deletes this node.
       */
      virtual ~BinarySearchTreeNode();

      /**
       *    @name Operators.
       *    The operators needed to keep the tree in order.
       */
      //@{
         /** 
           *   True if node's key is higher. 
           */
         virtual bool
            operator >  (const BinarySearchTreeNode &node) const = 0;

         /** 
           *   True if node's key is lower. 
           */
         virtual bool
            operator <  (const BinarySearchTreeNode &node) const = 0;

         /** 
           *   True if keys match.
           */
         virtual bool
            operator == (const BinarySearchTreeNode &node) const = 0;
      //@}

      /** 
       *    Get the node before this one. The one with closest lower key.
       *    @return  Node with closest lower key. 
       */
      BinarySearchTreeNode *pred() const;

      /** 
       *    Get the node after this one. The one with closest higher key.
       *    @return  Node with closest higher key.
       */
      BinarySearchTreeNode *suc() const;

      /** 
       *    Get the left child of this node.
       *    @return  This Node's left child.
       */
      BinarySearchTreeNode *getLeft() const;

      /** 
       *    Get the right child of this node.
       *    @return this Node's right child.
       */
      BinarySearchTreeNode *getRight() const;

   private:
      /**
       *    Make sure that the BinarySearchTree have access to all
       *    members and methods.
       */
      friend class BinarySearchTree;

      /**
        *   Find the selected node if in tree.
        *
        *   @param node Contains the key to search for.
        *   @return  The found element.
        */
      BinarySearchTreeNode *search(BinarySearchTreeNode *node) const;

      /** 
        *   Parent to the node.
        */
      BinarySearchTreeNode *parent;

      /** 
        *   Left child to the node.
        */
      BinarySearchTreeNode *left;

      /** 
        *   Right child to the node.
        */
      BinarySearchTreeNode *right;

      /** 
        *   Color true == red, false == black.
        */
      bool color;
};

/**
  *   Class for handling BinarySearchTrees.
  *   The tree is balanced by using red/black properties.
  *   NULL sentinels are used as leafs instead of NULL pointers.
  *   To walk through all nodes, find the node with lowest key with getMin() and 
  *   then use the suc() function of the node. Alt. use getMax() and pred().
  *
  *   @see BinarySearchTreeNode
  */
class BinarySearchTree: private NotCopyable {
   public:
      BinarySearchTree();

      virtual ~BinarySearchTree();

      /** 
        *   Add a node to the tree, keep tree balanced.
        *   @param x The node to add to this tree.
        */
      void add(BinarySearchTreeNode *x);

      /** 
        *   Remove a node from the tree, keep tree balanced.
        *   @param node The node to add to this tree.
        */
      void remove(BinarySearchTreeNode *node);

      /** 
        *   Get the node with minimum key in this tree.
        *   @return Node with minimum key.
        */
      BinarySearchTreeNode *getMin() const;

      /**
        *   Get the node with maximum key in this tree.
        *   @return node with maximum key. 
        */
      BinarySearchTreeNode *getMax() const;

      /** 
        *   Get the root of this tree.
        *   @return tree root.
        */
      BinarySearchTreeNode *getRoot() const;

      /** 
        *   Get the number of nodes in this tree.  
        *   @return This tree's cardinal.
        */
      uint32 getCardinal() const;

      /**
        *   Get a node with a given key.
        *   @param   node Node with prefered key.
        *   @return  Node matching selected key. (NULL if none found)
        */
      BinarySearchTreeNode *equal(BinarySearchTreeNode *node) const;

      /**
        *   Get a node with lower or equal key than the given one.
        *   @param   node Node with prefered key.
        *   @return  Node with key lower or equal to selected key.
        */
      BinarySearchTreeNode *lessOrEqual(BinarySearchTreeNode *node) const;

      /** 
        *   Get a node with higher or equal key than the given one.
        *   @param   node Node with prefered key.
        *   @return  Node with key equal or greater than selected key.
        */
      BinarySearchTreeNode *greaterOrEqual(BinarySearchTreeNode *node) const;

   private:
      /**
       *    Internal class that represents the snetinel in the tree.
       */
      class Sentinel : public BinarySearchTreeNode {
         public:
            /**
             *    @name Operators
             *    Overloaded operators to make sure we do not pass the 
             *    sentinel.
             */
            //@{
               /// Equal than (will always return false).
               bool operator >  (const BinarySearchTreeNode &node) const {
                  return false;
               }

               /// Less than (will always return false).
               bool operator <  (const BinarySearchTreeNode &node) const {
                  return false;
               }

               /// Equal to (will always return false).
               bool operator == (const BinarySearchTreeNode &node) const {
                  return false;
               }
            //@}
      };
   
		/** 
        *   Rotate node to the left. Used to balance tree. 
        *   @param   x  The node to rotate.
        */
      void leftRotate(BinarySearchTreeNode *x);

		/**
        *   Rotate node to the right. Used to balance tree. 
        *   @param   x  The node to rotate.
        */
      void rightRotate(BinarySearchTreeNode *x);

		/** 
        *   Rebalance tree after a delete has been made.
        *   @param   x  The node to fixup.
        */
      void deleteFixup(BinarySearchTreeNode *x);

		/** 
        *   Simple insert in tree, needs to be used with 
        *   balancing code. 
        *   @param   x  The node to insert.
        */
      void treeInsert(BinarySearchTreeNode *node);

      /** 
        *   Sentinel used to simplify tree modifications.
        */
      BinarySearchTreeNode *nullSentinel;

      /** 
        *   Root in the tree.
        */
      BinarySearchTreeNode *root;

      /** 
        *   Number of of items in tree. 
        */
      uint32 cardinal;
};

#endif

