/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GROUPITEM_H
#define GROUPITEM_H

#include "config.h"
#include "Item.h"

// Helper function to be able to item_cast this type of items.
inline GroupItem* item_cast_helper(Item* i, const GroupItem*)
{
   return Item::groupItem(i);
}

/**
  *   Objects of sub classes to this class is used to group other
  *   items. This is a very powerful way of grouping items, so it
  *   must be used with care. One example of grouping is StreetItem,
  *   that groups StreetSegmentItems into, what a human calls, streets.
  *   An other example is ZipCodeItem that groups a lot of
  *   StreetSegmentItems that have the same zip code.
  *
  */
class GroupItem : public Item {
public:
   /**
    *    Default construcor, implemented to be as fast as possible.
    */
   inline GroupItem() { };
      
   /**
    *   Creates an item containing information about a
    *   GroupItem. This constructor extracts (parses)
    *   the information from the dataBuffer.
    *
    *   @param   type  The type of this item.
    *   @param   id    The id of this item.
    */
   GroupItem(ItemTypes::itemType type, uint32 id);

   /**
    *    Destroy this group item.
    */
   ~GroupItem();


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
    *   Get the number of items in this group.
    *   @return The number of items in this group
    */
   inline uint32 getNbrItemsInGroup() const;
 
   /**
    *    Get the amount of memory used by this object.
    *    @return The size, in bytes, that this item uses.
    */
   uint32 getMemoryUsage() const;

   void setItemsInGroup( uint32* index ) { m_itemsInGroup = index; }

   inline const uint32* getItemsInGroup() const;

   /**
    *   Save this Item into a databuffer.
    *   @param   dataBuffer  The dataBuffer where this Item will
    *                        be saved.
    *   @param map the generic map
    */
   void save( DataBuffer& dataBuffer, const GenericMap& map ) const;

   /**
    *    Fill this item with information from the databuffer.
    *    @param   dataBuffer  The buffer with the data.
    *    @param theMap the generic map
    */
   void load(DataBuffer& dataBuffer, GenericMap& theMap);

private:
   friend class M3Creator;

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
GroupItem::getItemNumber(uint32 offset) const 
{
   if (offset < getNbrItemsInGroup()) {
      return m_itemsInGroup[offset];
   }
   return MAX_UINT32;
}

inline uint32 
GroupItem::getNbrItemsInGroup() const
{
   return m_nbrItemsInGroup;
}

inline const uint32* 
GroupItem::getItemsInGroup() const {
   return m_itemsInGroup;
}

#endif

