/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "OldNode.h"
#include "OldBusRouteItem.h"

#include "MC2String.h"
#include "StringUtility.h"
#include "DataBuffer.h"


OldBusRouteItem::OldBusRouteItem(uint32 id)
   : OldRouteableItem(ItemTypes::busRouteItem, id) 
{
   DEBUG_DB(mc2dbg << "OldBusRouteItem created" << endl);
   m_busRouteID = MAX_UINT32;
   m_offsetInClosestStreet = 0;
}

OldBusRouteItem::~OldBusRouteItem()
{
   DEBUG_DB(mc2dbg << "OldBusRouteItem destructed" << endl);
}

bool
OldBusRouteItem::save(DataBuffer *dataBuffer) const
{
   DEBUG_DB(mc2dbg << "      OldBusRouteItem::save()" << endl;)

   OldRouteableItem::save(dataBuffer);
   
   dataBuffer->writeNextLong(m_busRouteID);
   dataBuffer->writeNextShort(m_offsetInClosestStreet);
   dataBuffer->writeNextShort(0); // Pad 2 bytes

   return (true);
}

char* OldBusRouteItem::toString()
{
   char tmpStr[ITEM_AS_STRING_LENGTH];

   strcpy(tmpStr, OldRouteableItem::toString());
   sprintf(itemAsString,   "***** BusRouteItem\n%s"
                           "   busRouteID=%u\n"
                           "   offsetInClosestStreet=%u\n",
                           tmpStr,
                           m_busRouteID,
                           m_offsetInClosestStreet);
   return itemAsString;
}

uint32 
OldBusRouteItem::getMemoryUsage() const {
   uint32 totalSize = OldRouteableItem::getMemoryUsage()
                      - sizeof(OldRouteableItem) + sizeof(OldBusRouteItem);
                     
   return totalSize;
}

bool 
OldBusRouteItem::createFromDataBuffer(DataBuffer* dataBuffer,
                                   OldGenericMap* theMap)
{
   m_type = ItemTypes::busRouteItem;
   if (OldRouteableItem::createFromDataBuffer(dataBuffer, theMap)) {
      m_busRouteID = dataBuffer->readNextLong();
      m_offsetInClosestStreet = dataBuffer->readNextShort();
      dataBuffer->readNextShort(); // Pad 2 bytes
      return (true);
   } else {
      return (false);
   }
}



