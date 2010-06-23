/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CARTOGRAPHICITEM_H
#define CARTOGRAPHICITEM_H

#include "Item.h"
#include "ItemSubTypes.h"

ITEMCAST(CartographicItem, cartographicItem);

/**
 *   Item used for objects to be shown in the map, but not for routing, or as
 *   groups.
 */
class CartographicItem: public Item {
public:
   CartographicItem() {

   }
   explicit CartographicItem( uint32 id );

   ~CartographicItem() { 
   }

   /**
    *   Writes the item into the dataBuffer.
    *   @param   dataBuffer Where to print the information.
    *   @param map the generic map
    */
   void save( DataBuffer& dataBuffer, const GenericMap& map ) const;

   /**
    *    Fill this item with information from the databuffer.
    *    @param   dataBuffer  The buffer with the data.
    *    @param theMap the generic map
    */
   void load( DataBuffer& dataBuffer, GenericMap& theMap );

   /**
    *   @name Methods to get and set the building type attribute.
    */
   //@{
   /**
    *   Set the building type.
    *   @param   type  The building type.
    */
   inline void setCartographicType( ItemSubTypes::cartographicType_t cartographicType );
   
   /**
    *   Get the building type.
    *   @return The building type.
    */
   inline ItemSubTypes::cartographicType_t getCartographicType() const;
   //@}

private:
   /**
    *   The type of this building.
    */
   ItemSubTypes::cartographicType_t m_cartographicType;
};

void CartographicItem::
setCartographicType( ItemSubTypes::cartographicType_t type ) {
   m_cartographicType = type;
}

ItemSubTypes::cartographicType_t CartographicItem::
getCartographicType() const {
   return m_cartographicType;
}

#endif // CARTOGRAPHICITEM_H
