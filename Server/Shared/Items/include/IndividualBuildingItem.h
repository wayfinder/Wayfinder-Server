/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INDIVIDUALBUILDINGITEM_H
#define INDIVIDUALBUILDINGITEM_H

#include "Item.h"
#include "ItemSubTypes.h"

ITEMCAST(IndividualBuildingItem, individualBuildingItem);
/**
 *    One individual building on the map. That is an item where the 
 *    GfxData contains the actual outline of the building.
 *
 */
class IndividualBuildingItem : public Item {
public:
   /**
    *    Default construcor, implemented to be as fast as possible.
    */
   inline IndividualBuildingItem() { };

   /**
    *    Creates an item containing information about a
    *    IndividualBuildingItem.
    *    @param id The localID of this item.
    */
   explicit IndividualBuildingItem( uint32 id );

   /**
    *    Destroys this IndividualBuildningItem.
    */
   ~IndividualBuildingItem();

   /**
    *   @name Methods to get and set the building type attribute.
    */
   //@{
   /**
    *   Set the building type.
    *   @param   type  The building type.
    */
   inline void setBuildingType( ItemSubTypes::individualBuildingType_t type );
  
   /**
    *   Get the building type. 
    *   The building types are included in the points of interest types.
    *   @return The building type.
    */
   inline ItemSubTypes::individualBuildingType_t getBuildingType() const;
   //@}
      
   /**
    *    Writes the item into the dataBuffer.
    *    @param   dataBuffer Where to print the information.
    *    @param map the map
    *    @return  True if no errors, false otherwise.
    */
   void save( DataBuffer& dataBuffer, const GenericMap& map ) const;
      
   /**
    *    Fill this item with information from the databuffer.
    */
   void load( DataBuffer& dataBuffer, GenericMap& theMap );


private:
   /**
    *   The type (point of interest type) of this individual building.
    */
   ItemSubTypes::individualBuildingType_t m_buildingType;
};

// =======================================================================
//                                     Implementation of inlined methods =

inline void
IndividualBuildingItem::setBuildingType(
            ItemSubTypes::individualBuildingType_t type)
{
      m_buildingType = type;
}

inline ItemSubTypes::individualBuildingType_t
IndividualBuildingItem::getBuildingType() const
{
      return (m_buildingType);
}

#endif

