/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BUILTUPAREAITEM_H
#define BUILTUPAREAITEM_H

#include "config.h"
#include "GroupItem.h"

class BuiltUpAreaItem;

// Helper function to be able to item_cast this type of items.
inline BuiltUpAreaItem* item_cast_helper(Item* i,
                                         const BuiltUpAreaItem*)
{
   // Should be static_cast, but not known yet.
   return (BuiltUpAreaItem*)
      Item::itemOfType( i,
                        ItemTypes::builtUpAreaItem );
}


/**
  *   Describes one, what GDF calls, built up area.
  *
  */
class BuiltUpAreaItem : public GroupItem {
public:

   /**
    *    Get the dignity of this built up area. 
    *    @return  An estimated dignity of this built up area.
    */
   ItemTypes::city_t getBuiltUpAreaType();
      

   /**
    *    Get the number of inhabitants in this built up area.
    *    @return  The number of inhabitants in this BUA.
    *             Zero is returned if unknown.
    */
   inline uint32 getNbrInhabitants() const;

   /**
    *    Set the number of inhabitants in this built up area.
    *    Zero should be used to indicate unknown value.
    *    @param n  The number of inhabitants in this BUA.
    */
   inline void setNbrInhabitants(uint32 n);

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
   void load(DataBuffer& dataBuffer, GenericMap& theMap);

private:

   /**
    *    The number of inhabitants in this built up area. Set to zero
    *    if unknown.
    */
   uint32 m_nbrOfInhabitants;

   /**
    *    The display class of this BUA
    */
   ItemTypes::areaFeatureDrawDisplayClass_t m_displayClass : 4;
      

};

// =====================================================================
//                                   Implementation of inlined methods =

inline uint32
BuiltUpAreaItem::getNbrInhabitants() const
{
   return m_nbrOfInhabitants;
}

inline void
BuiltUpAreaItem::setNbrInhabitants(uint32 n)
{
   m_nbrOfInhabitants = n;
}

ItemTypes::areaFeatureDrawDisplayClass_t 
BuiltUpAreaItem::getDisplayClass() const {
   return m_displayClass;
}
#endif

