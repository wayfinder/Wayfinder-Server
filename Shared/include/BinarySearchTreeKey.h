/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BINARYSEARCHTREEKEY_H
#define BINARYSEARCHTREEKEY_H

#include "BinarySearchTree.h"

/**
  *   Binary search tree node with an uint32 as key.
  *
  *   @see BinarySearchTree
  */
class BinarySearchTreeKey : public BinarySearchTreeNode {

   public:
      /**
       *    Default constructor.
       */
      BinarySearchTreeKey() {};
      
      /**
       *    Constructor with key.
       *    @param   key   The key.
       */
      BinarySearchTreeKey(uint32 key) : m_key(key) {};
       
      /**
       *    Deletes this node.
       */
      virtual ~BinarySearchTreeKey() {};

      /**
       *    @name Operators.
       *    The operators needed to keep the tree in order.
       */
      //@{
         /** 
           *   True if node's key is higher. 
           */
         inline virtual bool
            operator >  (const BinarySearchTreeNode &node) const;

         /** 
           *   True if node's key is lower. 
           */
         inline virtual bool
            operator <  (const BinarySearchTreeNode &node) const;

         /** 
           *   True if keys match.
           */
         inline virtual bool
            operator == (const BinarySearchTreeNode &node) const;
      //@}
      
      /**
       *    @name Get/set key.
       *    Methods for setting and getting the key.
       */
      //@{
      
         /**
          * Set the key.
          * @param   key   The key.
          */
         inline void setKey(uint32 key);

         /**
          * Get the key.
          * @return The key.
          */
         inline uint32 getKey() const;

   private:

      /**
       * The key.
       */
      uint32 m_key;
};

// ========================================================================
//                                      Implementation of inlined methods =

inline bool
BinarySearchTreeKey::operator > (const BinarySearchTreeNode& node) const
{
   return (m_key > (static_cast<const BinarySearchTreeKey*> (&node))->m_key);
}

inline bool
BinarySearchTreeKey::operator < (const BinarySearchTreeNode& node) const
{
   return (m_key < (static_cast<const BinarySearchTreeKey*> (&node))->m_key);
}

inline bool
BinarySearchTreeKey::operator == (const BinarySearchTreeNode& node) const
{
   return (m_key == (static_cast<const BinarySearchTreeKey*> (&node))->m_key);
}

inline void
BinarySearchTreeKey::setKey(uint32 key)
{
   m_key = key;
}

inline uint32
BinarySearchTreeKey::getKey() const
{
   return (m_key);
}

#endif

