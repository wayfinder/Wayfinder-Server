/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "GroupItem.h"
#include "DataBuffer.h"
#include "GenericMap.h"

GroupItem::GroupItem(ItemTypes::itemType type, uint32 id) 
   : Item (type, id), m_itemsInGroup(NULL), m_nbrItemsInGroup(0)
{
   
}


GroupItem::~GroupItem()
{

}

void
GroupItem::save( DataBuffer& dataBuffer, const GenericMap& map ) const
{
   Item::save( dataBuffer, map );

   dataBuffer.writeNextLong( m_nbrItemsInGroup );
   if ( m_nbrItemsInGroup != 0 ) {
      dataBuffer.writeNextLong( m_itemsInGroup - map.getItemsInGroup() );
   }
}


void
GroupItem::load( DataBuffer& dataBuffer, GenericMap& theMap )
{
   Item::load( dataBuffer, theMap );

   m_nbrItemsInGroup = dataBuffer.readNextLong();

   if ( m_nbrItemsInGroup != 0 ) {
      m_itemsInGroup = const_cast<uint32*>(theMap.getItemsInGroup()) + 
         dataBuffer.readNextLong();
   }

}


uint32 
GroupItem::getMemoryUsage() const {
   return Item::getMemoryUsage() + m_nbrItemsInGroup * sizeof(uint32);
}

bool 
GroupItem::containsItem( uint32 itemID ) const
{
   uint32 i = 0;
   bool found = false;
   while ( ( i < getNbrItemsInGroup() ) && ( ! found ) ) {
      if ( getItemNumber( i ) == itemID ) {
         found = true;
      } else {
         i++;
      }
   }

   return found;
}

