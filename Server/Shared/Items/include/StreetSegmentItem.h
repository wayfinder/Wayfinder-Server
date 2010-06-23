/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STREETSEGMENTITEM_H
#define STREETSEGMENTITEM_H

#include "config.h"
#include "RouteableItem.h"

class DataBuffer;
class ItemNames;

ITEMCAST(StreetSegmentItem, streetSegmentItem);


/**
  *   Describes one streetsegment on the map. The streetsegment is 
  *   the smallest, logical part of a road. Have two nodes, one in
  *   each end of the segment.
  *
  */
class StreetSegmentItem : public RouteableItem {
public:
   /**
    *    Default construcor, implemented to be as fast as possible.
    */
   inline StreetSegmentItem() { MINFO("StreetSegmentItem()"); };

   /**
    *   Destroys the StreetSegmentItem.
    */
   ~StreetSegmentItem() {};

   /**
    *   Get the condition of the road.
    *   @return The road condition.
    */
   inline byte getRoadCondition() const;

   /**
    *   Set the class of this road to a new value.
    *   @param   c  The new road class.
    */
   inline void setRoadClass(int c);

   /**
    *   Get the class of this road.
    *   @return The road class (i.e. the level)
    */
   inline byte getRoadClass() const;
      
   /**
    *   @name Housenumber information.
    *   Methods to get infomration about the number of the houses on 
    *   this street segment. Left and right are when traversing the 
    *   segment from node 0 towards node 1.
    */
   //@{
   /**
    *   Get the type of the housenumbering on this street 
    *   segment.
    *   @return  Housnumber type on this street segment.
    */
   inline ItemTypes::streetNumberType getStreetNumberType() const;

   /**
    *
    */
   //@{
   /**
    *   Set the type of the housenumbering on this street.
    *   @param x Housnumber type on this street segment.
    */
   void setStreetNumberType(ItemTypes::streetNumberType x);
   //@}

   /**
    *   Get the width of this street segment. The width is returned
    *   in meters.
    *   @return the width of the road
    */
   inline uint8 getWidth() const;

   /**
    *   Find out if this street segment have any street as parent.
    *   That means that this method will return true if this
    *   street segment is part of any street (on higher zoom-level).
    *   @return  True if this street segment is part of any street,
    *            false otherwise.
    */
   inline bool partOfStreet() const;

   /**
    *   @name Roundabout-flag.
    *   These are the necessery methods to get and set the roundabout 
    *   attribute (if this segment is part of a roundabout of not).
    */
   //@{
   /**
    *   Set if this is a roundabout or not.
    *   @param   isRoundabout   True if part of a roundabout, 
    *                           false otherwise.
    */
   inline void setRoundaboutValue(bool isRoundabout);

   /**
    *   Check if this is a roundabout or not.
    *   @return  True if part of a roundabout, false otherwise.
    */
   inline bool isRoundabout() const;
   //@}
      
   /**
    *   @name Roundaboutish-flag. Traffic structure that is not a 
    *   roundabout but should give turndirections of the same kind, 
    *   i.e. "Take second exit".
    *   These are the necessery methods to get and set the 
    *   roundaboutish attribute 
    *   (if this segment is part of a roundaboutish traffic structure 
    *   or not).
    */
   //@{
   /**
    *   Set if this is roundaboutish or not.
    *   @param   isRoundaboutish   True if roundaboutish, 
    *                              false otherwise.
    */
   inline void setRoundaboutishValue(bool isRoundaboutish);

   /**
    *   Check if this is roundaboutish or not.
    *   @return  True if roundaboutish, false otherwise.
    */
   inline bool isRoundaboutish() const;
   //@}

   /**
    *   @name Ramp-attribute.
    *   Methods to get and set the ramp-attribute.
    */
   //@{
   /**
    *   Set if this is a ramp or not.
    *   @param   isRamp   True if part of a ramp, false otherwise.
    */
   inline void setRampValue(bool isRamp);

   /**
    *   Check if this is a ramp or not.
    *   @return  True if part of a ramp, false otherwise
    */
   inline bool isRamp() const;
   //@}
         
   /**
    *   @name Divided-attribute.
    *   Methods to get and set the divided-attribute.
    */
   //@{
   /**
    *   Set if this is streetsegment is divided or not.
    *   @param   isJunction  True if this SSI is divided,
    *                        false otherwise.
    */
   inline void setDividedValue(bool isDivided);

   /**
    *   Check if this is streetsegment is divided or not.
    *   @return True if this SSI is divided, false otherwise.
    */
   inline bool isDivided() const;
   //@}

   /**
    *   @name Multi digitalised-attribute.
    *   Methods to get and set the multi dig.-attribute.
    */
   //@{
   /**
    *   Set if this is streetsegment is multi digitised or not.
    *   @param   isMultiDig  True if this SSI is multi digitised,
    *                        false otherwise.
    */
   inline void setMultiDigitisedValue(bool isMultiDig);

   /**
    *   Check if this is streetsegment is multi digitised or not.
    *   @return True if this SSI is multi digitised, false otherwise.
    */
   inline bool isMultiDigitised() const;
   //@}

   /**
    *   @name Controlled access attribute.
    *   Methods to get and set the controlled access attribute.
    */
   //@{
   /**
    *   Set if this is a controlled access or not.
    *   @param isControlledAccess True if controlled access, 
    *                             false otherwise.
    */
   inline void setControlledAccessValue(bool isControlledAccess);

   /**
    *   Check if this is a controlled access or not.
    *   @return  True if controlled access, false otherwise
    */
   inline bool isControlledAccess() const;
   //@}
   
   /**
    *   @name Display class.
    *   Methods to get and set the display class for the road.
    */
   //@{
   /**
    *   Set the display class.
    *   @param displayClass The display class to set.
    */
   void setDisplayClass( ItemTypes::roadDisplayClass_t displayClass );

   /**
    *   Gets the display class.
    *   @return The display class.
    */
   inline ItemTypes::roadDisplayClass_t getDisplayClass() const;
   //@}
   
   /**
    *   Get the number of bytes this object uses of the memory.
    *   @return How many bytes this object uses.
    */
   uint32 getMemoryUsage() const;

   /**
    *   Writes the item into the dataBuffer.
    *   @param   dataBuffer Where to print the information.
    *   @return  True if no errors occurred, false otherwise.
    */
   void save( DataBuffer& dataBuffer, const GenericMap& map ) const;

   /**
    *   Create this street segment from a data buffer.
    *   @param   dataBuffer  The databuffer containing the data
    *                        for this street segment.
    */
   void load( DataBuffer& dataBuffer, GenericMap& theMap );
        
protected:
 
   /**
    *   Width of the road in meters.
    */
   byte m_width : 4;

   /**
    *   Type of enumeration of the houses.
    */
   ItemTypes::streetNumberType m_streetNumberType : 3;

   /**
    *   Class of road.
    */
   byte m_roadClass : 3;

   /**
    *   What condition the street is in.
    */
   byte m_condition : 2;

   /**
    *   True if this item is part of a roundabout, false otherwise.
    */
   bool  m_roundabout : 1;
      
   /**
    *   True if this item is roundaboutish, false otherwise.
    */
   bool  m_roundaboutish : 1;

   /**
    *   True if this item is part of a ramp, false otherwise.
    */
   bool m_ramp : 1;

   /**
    *   True if this streetsegment item is divided, false otherwise.
    */
   bool m_divided : 1;
      
   /**
    *   True if this streetsegment item is multi digitised, 
    *   (that is the road is described by more than one physical
    *   lane in GDF), false otherwise. (For instance parts of 
    *   Dalbyvägen and Norra ringen in Lund.)
    */
   bool m_multiDigitised : 1;

   /**
    *   True if this item is controlled access, false otherwise.
    */
   bool m_controlledAccess : 1;

   /**
    *   The display class of the road.
    */
   ItemTypes::roadDisplayClass_t m_displayClass : 4;
   
private:

   friend class M3Creator;
   /**
    *   @name Bit-masks
    *   Masks used when saving and loading the item from and to a 
    *   databuffer (the m_width, m_roundaboutish,
    *   m_roundabout, m_ramp, m_divided, 
    *   m_multiDigitised and m_controlledAccess membervariables).
    */
   //@{
   /// Use this to get the width.
   static const byte WIDTH_MASK;

   /// Use this to get the m_roundaboutish.
   static const byte ROUNDABOUTISH_MASK;
         
   /// Use this to get the streetnumber type.
   static const byte STREETNUMBERTYPE_MASK;

   /// Use this to get the m_roundabout.
   static const byte ROUNDABOUT_MASK;

   /// Use this to get the m_ramp.
   static const byte RAMP_MASK;

   /// Use this to get the m_divided.
   static const byte DIVIDED_MASK;

   /// Use this to get the m_multiDigitised.
   static const byte MULTI_DIGITISED_MASK;

   /// Use this to get the m_controlledAccess.
   static const byte CONTROLLED_ACCESS_MASK;
   //@}
      
};

// ========================================================================
//                                      Implementation of inlined methods =

inline byte 
StreetSegmentItem::getRoadCondition() const
{
   return m_condition;
}

inline void 
StreetSegmentItem::setRoadClass(int c)
{
   m_roadClass = c;
}

inline byte 
StreetSegmentItem::getRoadClass() const
{
   return m_roadClass;
}

inline ItemTypes::streetNumberType
StreetSegmentItem::getStreetNumberType () const
{
   return m_streetNumberType;
}

inline uint8 
StreetSegmentItem::getWidth() const
{
   return m_width;
}

inline bool 
StreetSegmentItem::partOfStreet() const
{
   bool retVal = false;
   uint32 myZoom = m_localID >> 27;
   for (uint32 i=0; i<getNbrGroups(); i++) {
      uint32 groupZoom = getGroup(i) >> 27;
      DEBUG4(cerr << "Item::partOfStreet() groupZoom = " 
                  << groupZoom << ", myZoom = " << myZoom << endl);
      if ( groupZoom < myZoom)
         retVal = true;
   }
   DEBUG4(
      if (retVal)
         cerr << "   Item::partOfStreet, returning TRUE" << endl;
      else
         cerr << "   Item::partOfStreet, returning FALSE" << endl;
   );

   return (retVal);
}

inline void 
StreetSegmentItem::setRoundaboutValue(bool isRoundabout) 
{
   m_roundabout = isRoundabout;
}

inline bool 
StreetSegmentItem::isRoundabout() const
{
   return (m_roundabout);
}

inline void 
StreetSegmentItem::setRoundaboutishValue(bool isRoundaboutish) 
{
   m_roundaboutish = isRoundaboutish;
}

inline bool 
StreetSegmentItem::isRoundaboutish() const
{
   return (m_roundaboutish);
}

inline void  
StreetSegmentItem::setRampValue(bool isRamp) 
{
   m_ramp = isRamp;
}

inline bool  
StreetSegmentItem::isRamp() const
{
   return (m_ramp);
}

inline void  
StreetSegmentItem::setDividedValue(bool isDivided) 
{
   m_divided = isDivided;
}

inline bool  
StreetSegmentItem::isDivided() const
{
   return (m_divided);
}

inline void 
StreetSegmentItem::setMultiDigitisedValue(bool isMultiDig) 
{
   m_multiDigitised = isMultiDig;
}

inline bool 
StreetSegmentItem::isMultiDigitised() const
{
   return (m_multiDigitised);
}

inline void  
StreetSegmentItem::setControlledAccessValue(bool isControlledAccess) 
{
   m_controlledAccess = isControlledAccess;
}

inline bool  
StreetSegmentItem::isControlledAccess() const
{
   return (m_controlledAccess);
}

ItemTypes::roadDisplayClass_t 
StreetSegmentItem::getDisplayClass() const {
   return m_displayClass;
}

#endif

