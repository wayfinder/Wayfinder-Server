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

#include "OldFerryItem.h"

#include "MC2String.h"
#include "StringUtility.h"
#include "DataBuffer.h"


OldFerryItem::OldFerryItem(uint32 id)
                  : OldRouteableItem(ItemTypes::ferryItem, id) 
{
   DEBUG_DB(mc2dbg << "OldFerryItem created" << endl);
   m_roadClass = 0;
   m_ferryType = 0;
}


OldFerryItem::~OldFerryItem()
{
   DEBUG_DB(mc2dbg << "OldFerryItem destructed" << endl);
}

bool
OldFerryItem::save(DataBuffer* dataBuffer) const
{
   DEBUG_DB(mc2dbg << "      OldFerryItem::save()" << endl;)

   OldRouteableItem::save(dataBuffer);
   dataBuffer->writeNextByte(m_roadClass);
   dataBuffer->writeNextByte(m_ferryType);
   
   return (true);
}

char* 
OldFerryItem::toString()
{
   char tmpStr[ITEM_AS_STRING_LENGTH];

   strcpy(tmpStr, OldRouteableItem::toString());
   sprintf(itemAsString,   "***** FerryItem\n%s",
                           tmpStr);
   return itemAsString;
}

uint32 
OldFerryItem::getMemoryUsage() const 
{
   uint32 totalSize = OldRouteableItem::getMemoryUsage()
      - sizeof(OldRouteableItem) + sizeof(OldFerryItem);
   
   return totalSize;
}


bool
OldFerryItem::createFromDataBuffer(DataBuffer* dataBuffer, 
                                OldGenericMap* theMap)
{
   m_type = ItemTypes::ferryItem;
   if (OldRouteableItem::createFromDataBuffer(dataBuffer, theMap)) {
      m_roadClass = dataBuffer->readNextByte();
      m_ferryType = dataBuffer->readNextByte();
      return (true);
   } else {
      return (false);
   }
}

bool
OldFerryItem::updateAttributesFromItem(OldItem* otherItem,
                                    bool sameMap)
{
   bool retVal = false;
   if ((otherItem == NULL) ||
       (otherItem->getItemType() != ItemTypes::ferryItem)) {
      return retVal;
   }
   
   // First general attributes such as names and gfxdata
   if (OldItem::updateAttributesFromItem(otherItem, sameMap))
      retVal = true;
   
   // Then routeable item specific attributes
   if (OldRouteableItem::updateAttributesFromItem(otherItem, sameMap))
      retVal = true;
   
   // Then item specific attributes
   OldFerryItem* otherFerry = (OldFerryItem*) otherItem;
   if (otherFerry != NULL) {

      // roadClass - don't update, connected with zoomlevel
      // ferry type
   
      if (m_ferryType != otherFerry->getFerryType()) {
         m_ferryType = otherFerry->getFerryType();
         mc2dbg4 << "    changing ferry type to " << int(m_ferryType) 
                 << " for ferry " << getID() << endl;
         retVal = true;
      }
   }

   return retVal;
}

