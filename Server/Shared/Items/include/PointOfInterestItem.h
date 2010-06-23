/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POINTOFINTERESTITEM_H
#define POINTOFINTERESTITEM_H

#include "config.h"
#include "Item.h"

// Helper function to be able to item_cast this type of items.
ITEMCAST(PointOfInterestItem, pointOfInterestItem);

/**
  *   Objects of this class describes point of intrests. These might
  *   be of several types, e.g. company or airport (described by the
  *   pointOfInterest_t) but all are located at one street segment.
  *
  */
class PointOfInterestItem : public Item {
public:
   /**
    *    Default construcor, implemented to be as fast as possible.
    */
   inline PointOfInterestItem() { };

   /**
    *   Get the ID of the StreetSegmentItem where this company is
    *   located.
    *   @return  ID to the item where this company is located.
    */
   inline uint32 getStreetSegmentItemID() const;

   /**
    *   Places the company at a new street.
    *   @param id new streetsegmentID
    */
   inline void setStreetSegmentItemID(uint32 id);

   /**
    *   Get the offset on streetSegmentItemID for the location of
    *   this company.
    *   @return  Offset on the streetSegmentID.
    */
   inline uint16 getOffsetOnStreet() const;

   /**
    *   Sets a new offset from the street's node0.
    *   @param offset new offset.
    */
   inline void setOffsetOnStreet(uint16 offset);

   /**
    *    Get WASP id for this POI.
    *    @return  The WASP id.
    */
   inline uint32 getWASPID() const;

   /**
    *    Set WASP id for this POI.
    *    @param   waspID   The WASP id.
    */
   void setWASPID(uint32 waspID);

   /**
    *   @name Methods to get information about the categories.
    */
   //@{
   /**
    *   Get the number of categories that this company is party of.
    *   @return  The number of categories this company "is in".
    */
   inline uint16 getNbrCategories() const;

   /**
    *   Get the id of one of the categories.
    *   @param   i  The index of the category to return. Valid values
    *               are 0 <= i < getNbrCategories().
    *   @return  ID of category number i. If no category with that
    *            number MAX_UINT32 is returned.
    */
   inline uint32 getCategory(uint16 i) const;
   //@}

   /**
    *   Compares the company with item.
    *
    *   @param   item The item to be compared with the company. If
    *            the dynamic qualification of this object isn't
    *            PointOfInterestItem or a subclass of
    *            PointOfInterestItem false is returned.
    *   @return  True if it appears to be the same company.
    */
   bool equals(PointOfInterestItem* item);

   /**
    *   Get the size of the memory used by this object.
    *   @return The number of bytes used by this object.
    */
   uint32 getMemoryUsage() const;

   /**
    *   @name Get, set and check the type of this point of interest.
    */
   //@{
   /**
    *   Get the type of this point of interest.
    *   @return  The type of this point of interest.
    */
   inline ItemTypes::pointOfInterest_t getPointOfInterestType() const;

   /**
    *   Set the type of this point of interest to a new value.
    *   @param   type  The new type of this point of interest.
    */
   inline void setPointOfInterestType(ItemTypes::pointOfInterest_t type);

   /**
    *   Check if this is a point of interest of a certain type.
    *   @param   type  The type to check against.
    *   @return  True if this point of interest is of type "type".
    */
   inline bool isPointOfInterestType(ItemTypes::pointOfInterest_t type) const;
   //@}

   /**
    *    Get the side on the street.
    *    @return The side on the street.
    */
   inline SearchTypes::side_t getSide() const;

   /**
    *    Set the side on the street.
    *    @param side The side on the street.
    */
   inline void setSide(SearchTypes::side_t side);

   /**
    *    Fill this item with information from the databuffer.
    *    @param   dataBuffer  The buffer with the data.
    *    @return  True if the data of the item is set, false
    *             otherwise.
    */
   void load( DataBuffer& dataBuffer, GenericMap& theMap );

   /**
    *   Writes the item into the dataBuffer.
    *   @param   dataBuffer Where to print the information.
    *   @return  True if no errors occurred, false otherwise.
    */
   void save( DataBuffer& dataBuffer, const GenericMap& map ) const;
      
private:

   uint32 m_waspID;

   /**
    *   The id of the street on which this company is located.
    */
   uint32 m_streetSegmentID;

   /**
    *   How far from the node0 this company is located.
    */
   uint16 m_offsetOnStreet;

   /**
    *   The type of this point of interest.
    */
   ItemTypes::pointOfInterest_t m_pointOfInterestType : 8;

   /**
    *  The side on the street of this company.
    */
   SearchTypes::side_t m_side : 8;

   /**
    *    Declaire Item as a friend, to make it possible to
    *    call the createFromDataBuffer-methd.
    */
   friend Item* Item::createNewItem( DataBuffer&, GenericMap& );
};

// ========================================================================
//                                      Implementation of inlined methods =

inline uint32
PointOfInterestItem::getStreetSegmentItemID() const
{
   return (m_streetSegmentID);
}

inline void
PointOfInterestItem::setStreetSegmentItemID(uint32 id)
{
   m_streetSegmentID = id;
}

inline uint16
PointOfInterestItem::getOffsetOnStreet() const
{
   return (m_offsetOnStreet);
}

inline void
PointOfInterestItem::setOffsetOnStreet(uint16 offset)
{
   m_offsetOnStreet = offset;
}

inline uint32 
PointOfInterestItem::getWASPID() const
{
   return m_waspID;
}

inline uint16
PointOfInterestItem::getNbrCategories() const
{
   uint16 retVal = 0;
   for (uint32 i=0; i<getNbrGroups(); i++) {
      if ((getGroup(i) & 0x78000000) == 0x78000000)
         retVal++;
   }
   return (retVal);
}

inline uint32
PointOfInterestItem::getCategory(uint16 i) const
{
   uint32 retVal = MAX_UINT32;
   uint32 j = 0;
   int foundGroups = -1;
   while ( (j < getNbrGroups()) &&
           (retVal == MAX_UINT32) ) {
      if ((getGroup(j) & 0x78000000) == 0x78000000)
         foundGroups++;
      if (foundGroups == (int) i)
         retVal = getGroup(j);
     j++;
   }
   return (retVal);
}

inline ItemTypes::pointOfInterest_t
PointOfInterestItem::getPointOfInterestType() const
{
   return (m_pointOfInterestType);
}

inline void
PointOfInterestItem::setPointOfInterestType(
      ItemTypes::pointOfInterest_t type)
{
   m_pointOfInterestType = type;
}

inline bool
PointOfInterestItem::isPointOfInterestType(
   ItemTypes::pointOfInterest_t type) const
{
   return (m_pointOfInterestType == type);
}

inline SearchTypes::side_t 
PointOfInterestItem::getSide() const
{
   return m_side;
}

inline void 
PointOfInterestItem::setSide(SearchTypes::side_t side) 
{
   m_side = side;
}


#endif

