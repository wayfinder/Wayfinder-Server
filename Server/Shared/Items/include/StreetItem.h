/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STREETITEM_H
#define STREETITEM_H

#include "config.h"
#include "GroupItem.h"

ITEMCAST(StreetItem, streetItem);

/**
  *   Describes one road, that is a collection of street segments.
  *
  */  
class StreetItem : public GroupItem {
public:

   ~StreetItem() {
      delete [] m_itemOffsetForPolygon;
   }

   /**
    *    Get the lowest item id present in the specified polygon.
    *    Look up the item in the map, and check it's properties
    *    in order to know what the properties for that polygon is.
    *    
    *    @param   polyIdx  The polygon index.
    *    @return  The item id if valid polygon, otherwise MAX_UINT32.
    */
   uint32 getItemInPolygon(uint16 polyIdx) const;
      
   /**
    *    Get the roadclass for the specified polygon.
    *    The caller of this method must make sure the indata is valid.
    *    This method relies on that the zoomlevel a ssi reflects
    *    the roadclass.
    *    
    *    @param   polyIdx  The polygon index.
    *    @return  The roadclass of that polygon. 
    */
   inline ItemTypes::roadClass getRoadClassForPolygon( uint16 polyIdx ) const;

   inline const uint16* getItemOffsetForPolygon() const;

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

   
protected:

   friend class M3Creator;


   /**
    *    Array which can be used to find the properties of a certain
    *    polygon (such as, the roadclass, is it a ramp, roundabout... etc)
    *    It has the same size as the number of polygons in the street.
    *    Index i corresponds to the i:th index in itemsInGroup member.
    *    In order to know what the properties of polygon i is, lookup
    *    itemsInGroup[m_itemOffsetForPolygon[i]] in the map. 
    *    The properties of that streetsegment item will be the same 
    *    as the properties for that polygon.
    */
   uint16* m_itemOffsetForPolygon;

};

// =======================================================================
//                                     Implementation of inlined methods =

inline ItemTypes::roadClass 
StreetItem::getRoadClassForPolygon( uint16 polyIdx ) const 
{
   return (ItemTypes::roadClass(
            GET_ZOOMLEVEL(
               getItemNumber(m_itemOffsetForPolygon[polyIdx])) - 2 ));
}

inline const uint16* 
StreetItem::getItemOffsetForPolygon() const 
{
   return m_itemOffsetForPolygon;
}

#endif

