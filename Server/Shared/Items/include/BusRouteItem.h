/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BUSROUTEITEM_H
#define BUSROUTEITEM_H

#include "config.h"
#include "RouteableItem.h"

ITEMCAST(BusRouteItem, busRouteItem);

/**
  *   Describes one bus route on the map. Timetables are stored in
  *   the InfoModule.
  *   
  */
class BusRouteItem : public RouteableItem {
public:
   /**
    *    Default construcor, implemented to be as fast as possible.
    */
   inline BusRouteItem() { };

   /**
    *   Destroys the BusRouteItem.
    */
   ~BusRouteItem() {};

   /**
    *   Get the number of bytes this object uses of the memory.
    *   @return How many bytes this object uses.
    */
   uint32 getMemoryUsage() const;

   /**
    *   Get offset in closest street from node 0.
    *   @return offset in closest street from node 0.
    */
   inline uint16 getOffsetInClosestStreet() const;

   /**
    *   Set offset in closest street from node 0.
    *   @param offset offset in closest street from node 0.
    */
   inline void setOffsetInClosestStreet(uint16 offset);

   /**
    *   Get bus route id. 
    *   @return Bus route id.
    */
   inline uint32 getBusRouteID() const;

   /**
    *   Set bus route id.
    *   @param id Bus route id.
    */
   inline void setBusRouteID(uint32 id);
         
   /**
    *   Writes the item into the dataBuffer.
    *   @param   dataBuffer Where to print the information.
    *   @param map the generic map
    */
   void save(DataBuffer& dataBuffer, const GenericMap& map ) const;

   /**
    *    Fill this item with information from the databuffer.
    *    @param   dataBuffer  The buffer with the data.
    *    @param theMap the generic map
    *             otherwise.
    */
   void load( DataBuffer& dataBuffer, GenericMap& theMap );

private:
   /**
    *   Bus route id.
    */
   uint32 m_busRouteID;

   /**
    *   Offset in closest street from node 0.
    *   Note that it would be better if this attribute was
    *   in a subclass to Node, however it is temporarily stored here.
    */
   uint16 m_offsetInClosestStreet;

};

// =======================================================================
//                                     Implementation of inlined methods =

inline uint16 
BusRouteItem::getOffsetInClosestStreet() const
{
   return m_offsetInClosestStreet;
}

inline void  
BusRouteItem::setOffsetInClosestStreet(uint16 offset) 
{
   m_offsetInClosestStreet = offset;
}

inline uint32  
BusRouteItem::getBusRouteID() const
{
   return m_busRouteID;
}

inline void  
BusRouteItem::setBusRouteID(uint32 id) 
{
   m_busRouteID = id;
}


#endif

