/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSFERRYITEM_H
#define GMSFERRYITEM_H

#include "OldFerryItem.h"
#include "GMSItem.h"
#include "GMSNode.h"
#include "GMSMap.h"

#include "AllocatorTemplate.h"


/**
  *   GenerateMapServer ferry item. Ferry item
  *   with extra features needed when creating the map.
  *
  */
class GMSFerryItem : public OldFerryItem,
                     public GMSItem {
   
   public:
      /**
        *   Constructor.
        *   @param   createNodes Whether to create nodes or not.
        *                        Default true.
        */
      GMSFerryItem(bool createNodes = true);
      
      /**
        *   Constructor with id.
        *   @param   id          The id of the item.
        *   @param   createNodes Whether to create nodes or not.
        *                        Default true.
        */
      GMSFerryItem(uint32 id, bool createNodes = true);

      /**
        *   Constructor with map type.
        *   @param   mapType     The type of the map.
        *   @param   createNodes Whether to create nodes or not.
        *                        Default true.
        */
      GMSFerryItem(GMSItem::map_t mapType, bool createNodes = true);

      /**
        *   Create a Ferry item with attributes from another ferry.
        *   @param   ferry   The other ferry.
        */
      GMSFerryItem(GMSFerryItem* ferry);

      /**
        *   Destructor.
        */
      virtual ~GMSFerryItem();

      /**
        *   Create a gms ferry item from mid/mif files.
        *   @warning Currently not implemented!
        *   @param midFile          The attribute file.
        *   @param readRestOfLine   If the rest of the mid line (eol) should 
        *                           be read or not, default true
        */
      bool createFromMidMif(ifstream& midFile, bool readRestOfLine = true);

      /**
        *   Get one of the nodes.
        *   @param   i  The node to return. Valid values are 0 for
        *               noe 0 and 1 for node 1.
        *   @return  Pointer to the node (GMSNode*) asked for (0 or 1).
        */
      inline GMSNode* getNode(int i);
      

   protected:
      
      /**
        *
        */
      virtual OldNode* createNewNode(DataBuffer* dataBuffer, uint32 nodeID) {
         DEBUG8(cerr << "Creating new GMSNode" << endl);
         return (new GMSNode(dataBuffer, nodeID));
      }

      virtual OldNode* getNewNode(OldGenericMap* theMap) {
         mc2dbg8 << here << "Creating new GMSNode" << endl;
         return (static_cast<MC2Allocator<GMSNode>*>
                            (theMap->m_nodeAllocator)->getNextObject());
      }

   private:

      /**
        *   Copy the attributes from another ferry to this ferry.
        *   @param   ferry   The other ferry.
        */
      void copyAttributes(GMSFerryItem* ferry);
      
      /**
       *    Init member variables of this ferry item.
       */
      void initMembers(uint32 id = MAX_UINT32, bool createNodes = true);
      
};

// ==================================================================
//                            Implementation of the inlined methods =

inline GMSNode* 
GMSFerryItem::getNode(int i)
{
   return (static_cast<GMSNode*> (OldRouteableItem::getNode(i)));
}

#endif
