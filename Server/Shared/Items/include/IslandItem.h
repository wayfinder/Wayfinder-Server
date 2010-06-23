/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ISLANDITEM_H
#define ISLANDITEM_H

#include "config.h"
#include "Item.h"

ITEMCAST(IslandItem, islandItem);

/**
  *   One island on the map.
  *
  */
class IslandItem : public Item {
public:
   /**
    *   @name Display class.
    *   Methods to get and set the display class for the water.
    */
   //@{
   /**
    *   Set the display class.
    *   @param displayClass The display class to set.
    */
   void setDisplayClass( ItemTypes::areaFeatureDrawDisplayClass_t displayClass );

   /**
    *   Gets the display class.
    *   @return The display class.
    */
   inline ItemTypes::areaFeatureDrawDisplayClass_t getDisplayClass() const;
   //@}

   /**
    *    Fill this item with information from the databuffer.
    *    @param   dataBuffer  The buffer with the data.
    *    @return  True if the data of the item is set, false
    *             otherwise.
    */
   void load( DataBuffer& dataBuffer, GenericMap& theMap );

private:
   /**
    *    The display class of the island.
    */
   ItemTypes::areaFeatureDrawDisplayClass_t m_displayClass : 4;
};

ItemTypes::areaFeatureDrawDisplayClass_t 
IslandItem::getDisplayClass() const {
   return m_displayClass;
}

#endif

