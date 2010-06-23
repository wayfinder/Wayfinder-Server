/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDGROUPITEM_H
#define OLDGROUPITEM_H

#include "config.h"
#include "OldItem.h"

/**
  *   Objects of sub classes to this class is used to group other
  *   items. This is a very powerful way of grouping items, so it
  *   must be used with care. One example of grouping is OldStreetItem,
  *   that groups OldStreetSegmentItems into, what a human calls, streets.
  *   An other example is OldZipCodeItem that groups a lot of
  *   OldStreetSegmentItems that have the same zip code.
  *
  */
class OldGroupItem : public OldItem {
   public:
      /**
       *    Default construcor, implemented to be as fast as possible.
       */
      inline OldGroupItem() { };
      
      /**
        *   Creates an item containing information about a
        *   OldGroupItem. This constructor extracts (parses)
        *   the information from the dataBuffer.
        *
        *   @param   type  The type of this item.
        *   @param   id    The id of this item.
        */
      OldGroupItem(ItemTypes::itemType type, uint32 id);

      /**
       *    Destroy this group item.
       */
      virtual ~OldGroupItem();

      /**
        *   Save this OldItem into a databuffer.
        *   @param   dataBuffer  The dataBuffer where this OldItem will
        *                        be saved.
        *   @return  True if all data saved into dataBuffer, false
        *            otherwise.
        */
      virtual bool save(DataBuffer* dataBuffer) const;

      /**
       *    Get the ID of one of the items in this group.
       *    @return Id of the offset:th item in this group.
       *            MAX_UINT32 is returned upon error.
       */
      inline uint32 getItemNumber(uint32 offset) const;

      /**
       *    Check if this item contains a specified item.
       *    @param   itemID   The item id of the item to look for.
       *    @return  True if this item contains the item with the
       *             specified item id, false otherwise.
       */
      bool containsItem( uint32 itemID ) const;

      /**
        *   This function adds an item to the group.
        *
        *   @param   groupID The group in which this item should be added.
        *   @return  true if the item was added to the group successfully,
        *            false otherwise.
        */
      inline bool addItem(uint32 itemID);

      /**
        *   Remove one item from this group.
        *   @param   pos   The position of the item to remove.
        *   @return  True if the item is removed, false otherwise.
        */
      inline bool removeItemNumber(uint32 pos);

      /**
        *   Remove one item from this group.
        *   @param   id    The ID of the item to remove.
        *   @return  True if the item is removed, false otherwise.
        */
      bool removeItemWithID(uint32 id);

      /**
        *   Get the number of items in this group.
        *   @return The number of items in this group
        */
      inline uint32 getNbrItemsInGroup() const;
 
      /**
       *   Writes data about the item into a class variable
       *   and returns a pointer to it. Not thread-safe
       *   @return The item in string form
       */
      char* toString();

      /**
       *    Get the amount of memory used by this object.
       *    @return The size, in bytes, that this item uses.
       */
      virtual uint32 getMemoryUsage() const;

   protected:
      /**
       *    Fill this item with information from the databuffer.
       *    @param   dataBuffer  The buffer with the data.
       *    @return  True if the data of the item is set, false
       *             otherwise.
       */
      virtual bool createFromDataBuffer(DataBuffer* dataBuffer, 
                                        OldGenericMap* theMap);

      /**
       *    Declaire OldItem as a friend, to make it possible to
       *    call the createFromDataBuffer-methd.
       */
      friend OldItem* OldItem::createNewItem(DataBuffer*, OldGenericMap*);
      friend class GMSItem;

   protected:
      /**
        *   Vector with the items in this group, contains 
        *   m_nbrItemsInGroup elements.
        */
      uint32* m_itemsInGroup;

      /**
       *    The number of elements in the m_itemsInGroup-array.
       */
      uint32 m_nbrItemsInGroup;

};

// =======================================================================
//                                     Implementation of inlined methods =

inline uint32 
OldGroupItem::getItemNumber(uint32 offset) const 
{
   if (offset < getNbrItemsInGroup()) {
      return m_itemsInGroup[offset];
   }
   return MAX_UINT32;
}

inline bool  
OldGroupItem::addItem(uint32 itemID) 
{
   uint32 nbrBefore = getNbrItemsInGroup();
   m_itemsInGroup = ArrayTool::addElement(m_itemsInGroup, 
                                        itemID, 
                                        m_nbrItemsInGroup);
   return ( nbrBefore != getNbrItemsInGroup() );
}

inline bool  
OldGroupItem::removeItemNumber(uint32 pos) 
{
   if (pos < getNbrItemsInGroup()) {
      ArrayTool::removeElement(m_itemsInGroup, pos, m_nbrItemsInGroup);
      return true;
   }
   return false;
}

inline uint32 
OldGroupItem::getNbrItemsInGroup() const
{
   return m_nbrItemsInGroup;
}



#endif

