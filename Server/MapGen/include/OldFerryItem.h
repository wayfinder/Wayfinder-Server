/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDFERRYITEM_H
#define OLDFERRYITEM_H

#include "OldRouteableItem.h"

/**
  *   Describes one ferry on the map.
  *      
  */
class OldFerryItem : public OldRouteableItem {
   public:
      /**
       *    Default construcor, implemented to be as fast as possible.
       */
      inline OldFerryItem() { };

      /**
        *   Creates an item with a specified ID. The two nodes
        *   are created, but they contains no information.
        *
        *   @param   id The localID of this item
        */
      OldFerryItem(uint32 id);

      /**
        *   Destroys the OldFerryItem.
        */
      virtual ~OldFerryItem();

      /**
        *   Writes the item into the dataBuffer.
        *   @param   dataBuffer Where to print the information.
        *   @return  True if no errors occurred, false otherwise.
        */
      virtual bool save(DataBuffer* dataBuffer) const;

      /**
        *   Set the roadclass of this ferry to a new value.
        *   @param   c  The new road class.
        */
      inline void setRoadClass(int c);
      
      /**
        *   Get the roadclass of this ferry.
        *   @return The road class (i.e. the importance of the ferry)
        */
      inline virtual byte getRoadClass() const;
      
      /**
        *   @name Ferry type.
        *   Methods to get and set the ferry type attribute.
        *   @see ItemTypes::ferryType.
        */
      //@{
         /**
           *   Set the ferry type.
           *   @param   type  The ferry type.
           */
         inline void setFerryType(byte type);

         /**
           *   Get the ferry type.
           *   @return The ferry type.
           */
         inline byte getFerryType() const;
      //@}
      
      /**
       *    Print (debug) information about the object into a
       *    static string in the class. Do not delete the string.
       *    @return The OldFerryItem as a string
       */
      virtual char* toString();

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
       *    Fill this item with information from the databuffer.
       *    @param   dataBuffer  The buffer with the data.
       *    @return  True if the data of the item is set, false
       *             otherwise.
       */
      virtual bool createFromDataBuffer(DataBuffer* dataBuffer, 
                                        OldGenericMap* theMap);

      /**
       *    Declaire OldItem as a friend, to make it possible to
       *    call the createFromDataBuffer-methd.
       */
      friend OldItem* OldItem::createNewItem(DataBuffer*, OldGenericMap*);
      friend class GMSItem;

      /**
        *   Class of road. Even though this is a ferry, it still
        *   has a roadclass that is comparable to the roadclass
        *   of a streetsegment.
        */
      byte m_roadClass;

      /**
       *    The type of ferry.
       *    @see ItemTypes::ferryType.
       */
      byte m_ferryType;
};

// ========================================================================
//                                      Implementation of inlined methods =
                                      
inline void 
OldFerryItem::setRoadClass(int c)
{
   m_roadClass=c;
}

inline byte 
OldFerryItem::getRoadClass() const
{
   return m_roadClass;
}  

inline void 
OldFerryItem::setFerryType(byte type) 
{
   m_ferryType = type;
}

inline byte 
OldFerryItem::getFerryType() const
{
   return (m_ferryType);
}


#endif

