/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTEABLEITEM_H
#define ROUTEABLEITEM_H

#include "config.h"
#include "Item.h"
#include "Node.h"

class GenericMap;

// Helper function to be able to item_cast this type of items.
inline RouteableItem* item_cast_helper(Item* i, const RouteableItem*)
{
   return Item::routeableItem(i);
}

/**
  *   Superclass of all routeable items. Consists of
  *   two nodes, one in each end of the segment.
  *
  */
class RouteableItem : public Item {
public:
   /**
    *    Default construcor, implemented to be as fast as possible.
    */
   inline RouteableItem() { MINFO("RouteableItem");};
      
   /**
    *   Destroys the RouteableItem.
    */
   ~RouteableItem() {};


     
   /**
    *   Get one of the nodes.
    *   @param   i  The node to return. Valid values are 0 for
    *               noe 0 and 1 for node 1.
    *   @return  Pointer to the node asked for (0 or 1).
    */
   inline Node* getNode(int i);

   /**
    *   Get one of the nodes when the node ID is supplied. If 
    *   the most significant bit is set, node 1 will be returned,
    *   otherwise node 0.
    *   @param   id The ID of the node to return. 
    *   @return  Pointer to the node indicated by the most
    *            significant bit in the id.
    */
   inline Node* getNodeFromID(uint32 id);

   byte getRoadClass() const;

   /**
    *   Get the number of bytes this object uses of the memory.
    *   @return How many bytes this object uses.
    */
   uint32 getMemoryUsage() const;

   /**
    *   Writes the item into the dataBuffer.
    *   @param   dataBuffer Where to print the information.
    *   @return  True if no errors occurred, false otherwise.
    */
   void save( DataBuffer& dataBuffer, const GenericMap& map ) const;

   /**
    *   Create this routeable item from a data buffer.
    *   @param   dataBuffer  The databuffer containing the data
    *                        for this street segment.
    */
   void load( DataBuffer& dataBuffer, GenericMap& theMap );
  
private:
   friend class M3Creator;
        
   /** 
    *   The first node of this routeable item.
    */
   Node m_node[2];

};

// ========================================================================
//                                      Implementation of inlined methods =

inline Node* 
RouteableItem::getNode(int i) 
{
   if (i == 0) {
      return (&m_node[0]);
   } else if (i == 1) {
      return (&m_node[1]);
   } else {
      return (NULL);
   }
}

inline Node* 
RouteableItem::getNodeFromID(uint32 id) 
{   
   if ( (id & 0x80000000) == 0x80000000) {
      return (&m_node[1]);
   } else {
      return (&m_node[0]);
   }
}


#endif

