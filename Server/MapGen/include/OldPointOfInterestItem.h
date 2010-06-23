/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDPOINTOFINTERESTITEM_H
#define OLDPOINTOFINTERESTITEM_H

#include "config.h"
#include "OldItem.h"

/**
  *   Objects of this class describes point of intrests. These might
  *   be of several types, e.g. company or airport (described by the
  *   pointOfInterest_t). All are located at one street segment.
  *
  */
class OldPointOfInterestItem : public OldItem {
   public:
      /**
       *    Default construcor, implemented to be as fast as possible.
       */
      inline OldPointOfInterestItem() { };

      /**
        *   Creates an item containing information about a
        *   OldPointOfInterestItem.
        *
        *   @param   id The localID of this item
        *   @param   type  The type of the new OldPointOfInterestItem.
        */
      OldPointOfInterestItem(
            uint32 id, 
            ItemTypes::pointOfInterest_t type = ItemTypes::unknownType);

      /**
        *   Destroys the OldPointOfInterestItem.
        */
      virtual ~OldPointOfInterestItem();

      /**
        *   Writes the item into the dataBuffer.
        *   @param   dataBuffer Where to print the information.
        *   @return  True if no errors occurred, false otherwise.
        */
      virtual bool save(DataBuffer* dataBuffer) const;

      /**
        *   Get the ID of the street segment item where this poi is
        *   located.
        *   @return  ID to the item where this poi is located.
        */
      inline uint32 getStreetSegmentItemID() const;

      /**
        *   Places the poi at a new street segment.
        *   @param id new streetsegmentID
        */
      inline void setStreetSegmentItemID(uint32 id);

      /**
        *   Get the offset on streetSegmentItemID for the location of
        *   this poi.
        *   @return  Offset on the streetSegmentID.
        */
      inline uint16 getOffsetOnStreet() const;

      /**
        *   Sets a new offset, counted from the street segment's node0.
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
        *   Compares the poi with another poi.
        *
        *   @param   item The item to be compared with the poi. If
        *            the dynamic qualification of this object isn't
        *            OldPointOfInterestItem or a subclass of
        *            OldPointOfInterestItem false is returned.
        *   @return  True if it appears to be the same poi.
        */
      bool equals(OldPointOfInterestItem* item);

      /**
        *   Writes data about the item into a class variable
        *   and returns a pointer to it. Not thread-safe
        *   @return The item in string form
        */
      char* toString();

      /**
        *   Get the size of the memory used by this object.
        *   @return The number of bytes used by this object.
        */
      virtual uint32 getMemoryUsage() const;

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
         inline void setPointOfInteresttype(
               ItemTypes::pointOfInterest_t type);

         /**
           *   Check if this is a point of interest of a certain type.
           *   @param   type  The type to check against.
           *   @return  True if this point of interest is of type "type".
           */
         inline bool isPointOfInterestType(
               ItemTypes::pointOfInterest_t type) const;
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
       *    Virtual method that updates all attributes of item and,
       *    if any, its nodes and connections to the values of the 
       *    other item. This includes e.g. names, groups, entry 
       *    restrictions, sign posts, speed limit and gfxdata.
       *    For more documentation see OldItem.h.
       *
       *    If item and other items originates from different maps
       *    the streetsegmentitem id, offset and side attributes are 
       *    not updated since they may be invalid in the map.
       *
       *    @param otherItem  The item from which to get update values.
       *    @param sameMap    Default true meaning that the items are
       *                      one and the same (same map), set to false if
       *                      the items originates from different maps (e.g.
       *                      underview and overview map)
       *    @return  True if some attribute was updated for the item,
       *             false if not.
       */
      virtual bool updateAttributesFromItem(OldItem* otherItem,
                                            bool sameMap = true);
      
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
        *   Reset the membervariables in this object.
        */
      virtual void init(ItemTypes::itemType type);


      /**
        *   The ID in the WASP POI database if the POI was added 
        *   to mcm map in WASPing.
        *   Initiated to MAX_UINT32, which is values for any POI that 
        *   was added to the mcm map in any other way.
        */
      uint32 m_waspID;

      /**
        *   The id of the street segment on which this poi is located.
        */
      uint32 m_streetSegmentID;

      /**
        *   How far from the node0 on the street segment this poi is located.
        */
      uint16 m_offsetOnStreet;

      /**
        *   The type of this point of interest.
        */
      ItemTypes::pointOfInterest_t m_pointOfInterestType : 8;

      /**
       *  The side on the street segment of this poi.
       */
      SearchTypes::side_t m_side : 8;

      /**
       *    Declaire OldItem as a friend, to make it possible to
       *    call the createFromDataBuffer-methd.
       */
      friend OldItem* OldItem::createNewItem(DataBuffer*, OldGenericMap*);
      friend class GMSItem;
};

// ========================================================================
//                                      Implementation of inlined methods =

inline uint32
OldPointOfInterestItem::getStreetSegmentItemID() const
{
   return (m_streetSegmentID);
}

inline void
OldPointOfInterestItem::setStreetSegmentItemID(uint32 id)
{
   m_streetSegmentID = id;
}

inline uint16
OldPointOfInterestItem::getOffsetOnStreet() const
{
   return (m_offsetOnStreet);
}

inline void
OldPointOfInterestItem::setOffsetOnStreet(uint16 offset)
{
   m_offsetOnStreet = offset;
}

inline uint32 
OldPointOfInterestItem::getWASPID() const
{
   return m_waspID;
}

inline ItemTypes::pointOfInterest_t
OldPointOfInterestItem::getPointOfInterestType() const
{
   return (m_pointOfInterestType);
}

inline void
OldPointOfInterestItem::setPointOfInteresttype(
      ItemTypes::pointOfInterest_t type)
{
   m_pointOfInterestType = type;
}

inline bool
OldPointOfInterestItem::isPointOfInterestType(
   ItemTypes::pointOfInterest_t type) const
{
   return (m_pointOfInterestType == type);
}

inline SearchTypes::side_t 
OldPointOfInterestItem::getSide() const
{
   return m_side;
}

inline void 
OldPointOfInterestItem::setSide(SearchTypes::side_t side) 
{
   m_side = side;
}


#endif

