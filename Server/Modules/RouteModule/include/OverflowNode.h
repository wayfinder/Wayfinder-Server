/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OVERFLOW_NODE_H
#define OVERFLOW_NODE_H

#include "config.h"
#include "CCSet.h"
#include "RoutingNode.h"

/**
 * A class describing an element in a linked list. The element stores an
 * overflow node in the BucketHeap. It is supposed to be used only rarely, as
 * overflow is never supposed to happen in the BucketHeap.
 *
 */
class OverflowNode : public Link
{
  public:

   /**
    * Constructor for the OverflowNode.
    * @param routingNode The RoutingNode that this object will hold.
    */
   inline OverflowNode(RoutingNode* routingNode);
   
   /**
    * Get the RoutingNode that this element contains.
    * @return The RoutingNode held by this linkage element.
    */
   inline RoutingNode* getNode() const;

   /**
    * Set the RoutingNode of this element.
    * @param routingNode The RoutingNode that this element should contain.
    */
   inline void setNode(RoutingNode* routingNode);

  private:

   /**
    * The RoutingNode of this element.
    */
   RoutingNode* m_routingNode;
   
}; // OverflowNode

///////////////////////////////////////////////////////////////////
// Inline functions
///////////////////////////////////////////////////////////////////

inline
OverflowNode::OverflowNode(RoutingNode* routingNode)
{
   m_routingNode = routingNode;
}


inline
RoutingNode* OverflowNode::getNode() const
{
   return m_routingNode;
}

inline void OverflowNode::setNode(RoutingNode* routingNode)
{
   m_routingNode = routingNode;
}

#endif
