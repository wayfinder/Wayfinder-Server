/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDSTREETSEGMENTITEM_H
#define OLDSTREETSEGMENTITEM_H

#include "config.h"
#include "OldRouteableItem.h"

class DataBuffer;
class OldItemNames;

/**
  *   Describes one streetsegment on the map. The streetsegment is 
  *   the smallest, logical part of a road. Have two nodes, one in
  *   each end of the segment.
  *
  */
class OldStreetSegmentItem : public OldRouteableItem {
   public:
      /**
       *    Default construcor, implemented to be as fast as possible.
       */
      inline OldStreetSegmentItem() { MINFO("OldStreetSegmentItem()"); };

      /**
        *   Creates an item containing information about a 
        *   OldStreetSegmentItem.
        *
        *   @param   id The localID of this item
        */
      OldStreetSegmentItem(uint32 id);

      /**
        *   Destroys the OldStreetSegmentItem.
        */
      virtual ~OldStreetSegmentItem() {};

      /**
        *   Writes the item into the dataBuffer.
        *   @param   dataBuffer Where to print the information.
        *   @return  True if no errors occurred, false otherwise.
        */
      virtual bool save(DataBuffer* dataBuffer) const;

      /**
       *    Print (debug) information about the object into a
       *    static string in the class. Do not delete the string.
       *    @return The OldStreetSegmentItem as a string
       */
      virtual char* toString();

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
      inline virtual byte getRoadClass() const;
      
      /**
        *   Writes the street segment item specific header to the mifFile.
        *   @param mifFile   The mif-file to write header to.
        */
      static void writeMifHeader(ofstream& mifFile);

      /**
        *  Writes a street segment item to a mid and mif file.
        *  @param  mifFile File to write gfx-data to.
        *  @param  midFile File to write attributes in.
        *  @param  namePointer Pointer to stringtable.
        */
      virtual void printMidMif(ofstream& midFile, ofstream& mifFile, 
                               OldItemNames* namePointer);
      
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
           *   Get the first street number on the left side.
           *   @return The start street nbr on the left side
           */
         inline uint16 getLeftSideNbrStart() const;

         /**
           *   Get the last street number on the left side.
           *   @return The end street nbr on the left side
           */
         inline uint16 getLeftSideNbrEnd() const;

         /**
           *   Get the first street number on the right side.
           *   @return The start street nbr on the right side
           */
         inline uint16 getRightSideNbrStart() const;

         /**
           *   Get the last street number on the right side.
           *   @return The end street nbr on the right side
           */
         inline uint16 getRightSideNbrEnd() const;

         /**
           *   Get one side in the street number interval.
           *   @param   left Is the streetnumber on the left side?
           *   @param   start Is it the first streetnumber?
           *   @return  The streetnumber that matches the parameters.
           */
         inline uint16 getStreetNumber(bool left, bool start) const;
      //@}
      
      /**
       *
       */
      //@{
         /**
           *   Set the type of the housenumbering on this street.
           *   @param x Housnumber type on this street segment.
           */
         void setStreetNumberType(ItemTypes::streetNumberType x);
         /**
           *   Set the first street number on the left side.
           *   @param t The start street nbr on the left side
           */
         void setLeftSideNumberStart(uint16 t);

         /**
           *   Set the last street number on the left side.
           *   @param t The end street nbr on the left side
           */
         void setLeftSideNumberEnd(uint16 t);

         /**
           *   Set the first street number on the right side.
           *   @param t The start street nbr on the right side
           */
         void setRightSideNumberStart(uint16 t);

         /**
           *   Set the last street number on the right side.
           *   @param t The end street nbr on the right side
           */
         void setRightSideNumberEnd(uint16 t);
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
        *   Get the number of bytes this object uses of the memory.
        *   @return How many bytes this object uses.
        */
      virtual uint32 getMemoryUsage() const;

      /**
       *    Virtual method that updates all attributes of item and,
       *    if any, its nodes and connections to the values of the 
       *    other item. This includes e.g. names, groups, entry 
       *    restrictions, sign posts, speed limit and gfxdata.
       *    For more documentation see OldItem.h.
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
        *   Create this street segment from a data buffer.
        *   @param   dataBuffer  The databuffer containing the data
        *                        for this street segment.
        */
      virtual bool createFromDataBuffer(DataBuffer* dataBuffer, 
                                        OldGenericMap* theMap);
      
      /**
        *   @name Streetnumbers
        *   Members to store the housenumbers on this segment.
        *
        *   @{
        */
         /// First number on the left side.
         uint16 m_leftsideNumberStart;

         /// Last number on the left side.
         uint16 m_leftsideNumberEnd;

         /// First number on the right side.
         uint16 m_rightsideNumberStart;

         /// Last number on the right side.
         uint16 m_rightsideNumberEnd;
      /** @} */

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
        *   lane, false otherwise.
        */
      bool m_multiDigitised : 1;

       /**
        *   True if this item is controlled access, false otherwise.
        */
      bool m_controlledAccess : 1;     
   
   private:
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
      
      /**
       *    Declaire OldItem as a friend, to make it possible to
       *    call the createFromDataBuffer-methd.
       */
      friend OldItem* OldItem::createNewItem(DataBuffer*, OldGenericMap*);
      
      friend class GMSItem;

};

// ========================================================================
//                                      Implementation of inlined methods =

inline byte 
OldStreetSegmentItem::getRoadCondition() const
{
   return m_condition;
}

inline void 
OldStreetSegmentItem::setRoadClass(int c)
{
   m_roadClass = c;
}

inline byte 
OldStreetSegmentItem::getRoadClass() const
{
   return m_roadClass;
}

inline ItemTypes::streetNumberType
OldStreetSegmentItem::getStreetNumberType () const
{
   return m_streetNumberType;
}

inline uint16  
OldStreetSegmentItem::getLeftSideNbrStart() const
{
   return m_leftsideNumberStart;
}

inline uint16  
OldStreetSegmentItem::getLeftSideNbrEnd() const
{
   return m_leftsideNumberEnd;
}

inline uint16  
OldStreetSegmentItem::getRightSideNbrStart() const
{
   return m_rightsideNumberStart;
}

inline uint16  
OldStreetSegmentItem::getRightSideNbrEnd() const
{
   return m_rightsideNumberEnd;
}

inline uint16  
OldStreetSegmentItem::getStreetNumber(bool left, bool start) const
{
   if (left) {
      if (start)
         return m_leftsideNumberStart;
      else
         return m_leftsideNumberEnd;
   } else {
      if (start)
         return m_rightsideNumberStart;
      else
         return m_rightsideNumberEnd;
   }
}

inline uint8 
OldStreetSegmentItem::getWidth() const
{
   return m_width;
}

inline bool 
OldStreetSegmentItem::partOfStreet() const
{
   bool retVal = false;
   uint32 myZoom = m_localID >> 27;
   for (uint32 i=0; i<getNbrGroups(); i++) {
      uint32 groupZoom = getGroup(i) >> 27;
      DEBUG4(cerr << "OldItem::partOfStreet() groupZoom = " 
                  << groupZoom << ", myZoom = " << myZoom << endl);
      if ( groupZoom < myZoom)
         retVal = true;
   }
   DEBUG4(
      if (retVal)
         cerr << "   OldItem::partOfStreet, returning TRUE" << endl;
      else
         cerr << "   OldItem::partOfStreet, returning FALSE" << endl;
   );

   return (retVal);
}

inline void 
OldStreetSegmentItem::setRoundaboutValue(bool isRoundabout) 
{
   m_roundabout = isRoundabout;
}

inline bool 
OldStreetSegmentItem::isRoundabout() const
{
   return (m_roundabout);
}

inline void 
OldStreetSegmentItem::setRoundaboutishValue(bool isRoundaboutish) 
{
   m_roundaboutish = isRoundaboutish;
}

inline bool 
OldStreetSegmentItem::isRoundaboutish() const
{
   return (m_roundaboutish);
}

inline void  
OldStreetSegmentItem::setRampValue(bool isRamp) 
{
   m_ramp = isRamp;
}

inline bool  
OldStreetSegmentItem::isRamp() const
{
   return (m_ramp);
}

inline void  
OldStreetSegmentItem::setDividedValue(bool isDivided) 
{
   m_divided = isDivided;
}

inline bool  
OldStreetSegmentItem::isDivided() const
{
   return (m_divided);
}

inline void 
OldStreetSegmentItem::setMultiDigitisedValue(bool isMultiDig) 
{
   m_multiDigitised = isMultiDig;
}

inline bool 
OldStreetSegmentItem::isMultiDigitised() const
{
   return (m_multiDigitised);
}

inline void  
OldStreetSegmentItem::setControlledAccessValue(bool isControlledAccess) 
{
   m_controlledAccess = isControlledAccess;
}

inline bool  
OldStreetSegmentItem::isControlledAccess() const
{
   return (m_controlledAccess);
}


#endif

