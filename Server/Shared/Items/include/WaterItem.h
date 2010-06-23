/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WATERITEM_H
#define WATERITEM_H

#include "config.h"
#include "Item.h"

ITEMCAST(WaterItem, waterItem);

/**
  *   Describes one piece of water on the map.
  *
  */
class WaterItem : public Item {
public:
   /**
    *    Default construcor, implemented to be as fast as possible.
    */
   inline WaterItem() { };

   /**
    *   Creates an item containing information about a
    *   WaterItem.
    *
    *   @param   id The localID of this item
    */
   explicit WaterItem( uint32 id );

   /**
    *   Destroys the StreetSegmentItem.
    */
   ~WaterItem();

   /**
    *   @name Methods to get and set the water type attribute.
    */
   //@{
   /**
    *   Set the water type.
    *   @param   type  The water type.
    */
   inline void setWaterType(byte type);

   /**
    *   Get the water type.
    *   @return The water type.
    */
   inline byte getWaterType() const;
   //@}
   
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
    *   Writes the item into the dataBuffer.
    *   @param   dataBuffer Where to print the information.
    *   @return  True if no errors occurred, false otherwise.
    */
   void save( DataBuffer& dataBuffer, const GenericMap& map ) const;

   /**
    *    Fill this item with information from the databuffer.
    *    @param   dataBuffer  The buffer with the data.
    *    @return  True if the data of the item is set, false
    *             otherwise.
    */
   void load( DataBuffer& dataBuffer, GenericMap& theMap );

private:
   /**
    *    Water type.
    */
   byte m_waterType;

   /**
    *    The display class of this water.
    */
   ItemTypes::areaFeatureDrawDisplayClass_t m_displayClass : 4;
};

// ========================================================================
//                                      Implementation of inlined methods =

inline void 
WaterItem::setWaterType(byte type) 
{
   m_waterType = type;
}

inline byte 
WaterItem::getWaterType() const 
{
   return (m_waterType);
}

ItemTypes::areaFeatureDrawDisplayClass_t 
WaterItem::getDisplayClass() const {
   return m_displayClass;
}

#endif

