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
#include "OldPointOfInterestItem.h"
#include "DataBuffer.h"

OldPointOfInterestItem::OldPointOfInterestItem(uint32 id,
                                         ItemTypes::pointOfInterest_t type)
   : OldItem(ItemTypes::pointOfInterestItem, id)
{
   // Reset all the members
   init(ItemTypes::pointOfInterestItem);

   // Set the type of point of interest
   m_pointOfInterestType = type;
}

OldPointOfInterestItem::~OldPointOfInterestItem()
{
   // Nothing is allocated here
}

void
OldPointOfInterestItem::init(ItemTypes::itemType type)
{
   OldItem::init(type);

   m_streetSegmentID = MAX_UINT32;
   m_offsetOnStreet = 0;

   m_pointOfInterestType = ItemTypes::company;
   m_side = SearchTypes::undefined_side;
   m_waspID = MAX_UINT32;
}

void
OldPointOfInterestItem::setWASPID(uint32 waspID)
{
   m_waspID = waspID;
}


bool
OldPointOfInterestItem::save(DataBuffer* dataBuffer) const
{
   DEBUG_DB(mc2dbg << "OldPointOfInterestItem::save()" << endl;)

   OldItem::save(dataBuffer);

   dataBuffer->writeNextByte(m_pointOfInterestType);
   dataBuffer->writeNextByte(m_side);
   dataBuffer->writeNextShort(m_offsetOnStreet);
   DEBUG_DB(mc2dbg << "POItype = " << uint32(m_pointOfInterestType) << endl;);
   DEBUG_DB(mc2dbg << "side = " << uint32(m_side) << endl;);
   DEBUG_DB(mc2dbg << "offsetOnStreet = " << m_offsetOnStreet << endl);

   dataBuffer->writeNextLong(m_streetSegmentID);
   DEBUG_DB(mc2dbg << "streetSegmentID = " << m_streetSegmentID << endl);

   dataBuffer->writeNextLong(m_waspID);
   DEBUG_DB(mc2dbg << "WASP = " << m_waspID << endl);

   return (true);
}

bool
OldPointOfInterestItem::equals(OldPointOfInterestItem* item)
{
   if ((item != NULL) &&
       ( (  (  (item->getNbrNames() == 0) &&
               (this->getNbrNames() == 0) ) ||
            (this->getStringIndex(0) == item->getStringIndex(0))
       ) ) &&
       (this->m_streetSegmentID    == item->m_streetSegmentID) &&
       (this->m_waspID == item->m_waspID) &&
       (this->m_offsetOnStreet == item->m_offsetOnStreet)) {
      return (true);
   } else {
      return (false);
   }
}

char* OldPointOfInterestItem::toString()
{
   char tmpStr[1024];
   strcpy(tmpStr, OldItem::toString());
   sprintf(itemAsString,   "***** PointOfInterestItem\n"
                           "%s"
                           "streetSegmentID=%u\n"
                           "offsetOnStreet=%u\n"
                           "waspID=%u\n",
                           tmpStr,
                           m_streetSegmentID,
                           m_offsetOnStreet,
                           m_waspID);
   return itemAsString;
}

uint32
OldPointOfInterestItem::getMemoryUsage() const
{
   return OldItem::getMemoryUsage() - sizeof(OldItem) + sizeof(OldPointOfInterestItem);
}

bool
OldPointOfInterestItem::createFromDataBuffer(DataBuffer *dataBuffer,
                                          OldGenericMap* theMap)
{
   // Set the type and create the OldItem-part
   m_type = ItemTypes::pointOfInterestItem;
   if (OldItem::createFromDataBuffer(dataBuffer, theMap)) {
      m_pointOfInterestType = 
         ItemTypes::pointOfInterest_t(dataBuffer->readNextByte());
      m_side = SearchTypes::side_t( dataBuffer->readNextByte() );
      m_offsetOnStreet = dataBuffer->readNextShort();
      DEBUG_DB(mc2dbg << "   POItype = " << uint16(m_pointOfInterestType) 
                    << endl;);
      DEBUG_DB(mc2dbg << "   side = " << uint16(m_side) << endl;);
      DEBUG_DB(mc2dbg << "   offsetOnStreet=" << m_offsetOnStreet << endl);

      m_streetSegmentID = dataBuffer->readNextLong();
      DEBUG_DB(mc2dbg << "   streetSegmentID = " << m_streetSegmentID << endl);

      m_waspID = dataBuffer->readNextLong();
      DEBUG_DB(mc2dbg << "   waspID = " << m_waspID << endl);
      
      return (true);
   } else {
      return (false);
   }
}

bool
OldPointOfInterestItem::updateAttributesFromItem(
                  OldItem* otherItem, bool sameMap)
{
   bool retVal = false;
   mc2dbg4 << "OldPointOfInterestItem::updateAttributesFromItem" << endl;
   if ((otherItem == NULL) ||
       (otherItem->getItemType() != ItemTypes::pointOfInterestItem)) {
      return retVal;
   }

   OldPointOfInterestItem* otherPoi = (OldPointOfInterestItem*) otherItem;
   if (otherPoi != NULL) {

      if (m_waspID != otherPoi->getWASPID()) {
         mc2log << error << "Updating poi-attributes, but waspId does "
                << " not match thisPoi=" << m_waspID << " otherPoi="
                << otherPoi->getWASPID() << endl;
         return retVal;
      }

      // First general attributes such as names and gfxdata
      if (OldItem::updateAttributesFromItem(otherItem, sameMap))
         retVal = true;
   
      // Then item specific attributes
      // wasp id - must be equal, else it is not the same poi!
      // poi type
      // ssi id
      // offset
      // side
      
      if (!otherPoi->isPointOfInterestType(m_pointOfInterestType)) {
         m_pointOfInterestType = otherPoi->getPointOfInterestType();
         retVal = true;
      }

      if ( sameMap ) {
         // Ssi id, offset and side are only valid if the items 
         // originate from the same map (the items are one and the same)
         
         if (m_streetSegmentID != otherPoi->getStreetSegmentItemID()) {
            m_streetSegmentID = otherPoi->getStreetSegmentItemID();
            retVal = true;
         }
         
         if (m_offsetOnStreet != otherPoi->getOffsetOnStreet()) {
            m_offsetOnStreet = otherPoi->getOffsetOnStreet();
            retVal = true;
         }
         
         if (m_side != otherPoi->getSide()) {
            m_side = otherPoi->getSide();
            retVal = true;
         }
      }
   }
   
   return retVal;
}

