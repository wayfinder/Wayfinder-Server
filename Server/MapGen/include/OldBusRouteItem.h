/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDBUSROUTEITEM_H
#define OLDBUSROUTEITEM_H

#include "config.h"
#include "OldRouteableItem.h"

/**
  *   Describes one bus route on the map. Timetables are stored in
  *   the InfoModule.
  *   
  */
class OldBusRouteItem : public OldRouteableItem {
   public:
      /**
       *    Default construcor, implemented to be as fast as possible.
       */
      inline OldBusRouteItem() { };

      /**
        *   Creates an item with a specified ID. The two nodes
        *   are created, but they contains no information.
        *
        *   @param   id The localID of this item
        */
      OldBusRouteItem(uint32 id);

      /**
        *   Destroys the OldBusRouteItem.
        */
      virtual ~OldBusRouteItem();

      /**
        *   Writes the item into the dataBuffer.
        *   @param   dataBuffer Where to print the information.
        *   @return  True if no errors occurred, false otherwise.
        */
      virtual bool save(DataBuffer *dataBuffer) const;

      /**
       *    Print (debug) information about the object into a
       *    static string in the class. Do not delete the string.
       *    @return The OldBusRouteItem as a string
       */
      virtual char* toString();

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
         
   protected:
      /**
       *    Fill this item with information from the databuffer.
       *    @param   dataBuffer  The buffer with the data.
       *    @return  True if the data of the item is set, false
       *             otherwise.
       */
      virtual bool createFromDataBuffer(DataBuffer* dataBuffer, 
                                        OldGenericMap* theMap);

   private:
      /**
        *   Bus route id.
        */
      uint32 m_busRouteID;

      /**
        *   Offset in closest street from node 0.
        *   Note that it would be better if this attribute was
        *   in a subclass to OldNode, however it is temporarily stored here.
        */
      uint16 m_offsetInClosestStreet;

      /**
       *    Declaire OldItem as a friend, to make it possible to
       *    call the createFromDataBuffer-methd.
       */
      friend OldItem* OldItem::createNewItem(DataBuffer*, OldGenericMap*);
      friend class GMSItem;
 };

// =======================================================================
//                                     Implementation of inlined methods =

inline uint16 
OldBusRouteItem::getOffsetInClosestStreet() const
{
   return (m_offsetInClosestStreet);
}

inline void  
OldBusRouteItem::setOffsetInClosestStreet(uint16 offset) 
{
   m_offsetInClosestStreet = offset;
}

inline uint32  
OldBusRouteItem::getBusRouteID() const
{
   return (m_busRouteID);
}

inline void  
OldBusRouteItem::setBusRouteID(uint32 id) 
{
   m_busRouteID = id;
}


#endif

