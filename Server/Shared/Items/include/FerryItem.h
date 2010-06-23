/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FERRYITEM_H
#define FERRYITEM_H

#include "RouteableItem.h"

ITEMCAST(FerryItem, ferryItem);

/**
  *   Describes one ferry on the map.
  *      
  */
class FerryItem : public RouteableItem {
public:
   /**
    *    Default construcor, implemented to be as fast as possible.
    */
   inline FerryItem() { };

   /**
    *   Destroys the FerryItem.
    */
   ~FerryItem() {};

   /**
    *   Set the roadclass of this ferry to a new value.
    *   @param   c  The new road class.
    */
   inline void setRoadClass(int c);
      
   /**
    *   Get the roadclass of this ferry.
    *   @return The road class (i.e. the importance of the ferry)
    */
   inline byte getRoadClass() const;
      
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
    *   Get the number of bytes this object uses of the memory.
    *   @return How many bytes this object uses.
    */
   uint32 getMemoryUsage() const;
         
   /**
    *   Writes the item into the dataBuffer.
    *   @param   dataBuffer Where to print the information.
    *   @param map the map
    */
   void save( DataBuffer& dataBuffer, const GenericMap& map ) const;

   /**
    *    Fill this item with information from the databuffer.
    *    @param   dataBuffer  The buffer with the data.
    *    @param theMap map
    */
   void load( DataBuffer& dataBuffer, GenericMap& theMap );

protected:

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
FerryItem::setRoadClass(int c)
{
   m_roadClass=c;
}

inline byte 
FerryItem::getRoadClass() const
{
   return m_roadClass;
}  

inline void 
FerryItem::setFerryType(byte type) 
{
   m_ferryType = type;
}

inline byte 
FerryItem::getFerryType() const
{
   return (m_ferryType);
}


#endif

