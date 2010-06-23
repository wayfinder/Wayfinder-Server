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
#include "PointOfInterestItem.h"
#include "DataBuffer.h"

void
PointOfInterestItem::setWASPID(uint32 waspID)
{
   m_waspID = waspID;
}


void
PointOfInterestItem::save( DataBuffer& dataBuffer, const GenericMap& map) const
{
   Item::save( dataBuffer, map);

   dataBuffer.writeNextByte( m_pointOfInterestType );
   dataBuffer.writeNextByte( m_side );
   dataBuffer.writeNextShort( m_offsetOnStreet );
   dataBuffer.writeNextLong( m_streetSegmentID );
   dataBuffer.writeNextLong( m_waspID );

}

void
PointOfInterestItem::load( DataBuffer& dataBuffer, GenericMap& theMap )
{

   m_type = ItemTypes::pointOfInterestItem;
   Item::load( dataBuffer, theMap );

   m_pointOfInterestType = 
      ItemTypes::pointOfInterest_t(dataBuffer.readNextByte());
   m_side = SearchTypes::side_t( dataBuffer.readNextByte() );
   m_offsetOnStreet = dataBuffer.readNextShort();
   m_streetSegmentID = dataBuffer.readNextLong();
   m_waspID = dataBuffer.readNextLong();
     
}

bool
PointOfInterestItem::equals(PointOfInterestItem* item)
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

uint32
PointOfInterestItem::getMemoryUsage() const
{
   return Item::getMemoryUsage() - sizeof(Item) + sizeof(PointOfInterestItem);
}

