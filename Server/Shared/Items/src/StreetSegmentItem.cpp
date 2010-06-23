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

#include "Node.h"
#include "StreetSegmentItem.h"

#include "MC2String.h"
#include "DataBuffer.h"


const byte StreetSegmentItem::WIDTH_MASK = 0x0f;

const byte StreetSegmentItem::ROUNDABOUTISH_MASK = 0x10;

const byte StreetSegmentItem::STREETNUMBERTYPE_MASK = 0x07;

const byte StreetSegmentItem::ROUNDABOUT_MASK = 0x08;

const byte StreetSegmentItem::RAMP_MASK = 0x10;

const byte StreetSegmentItem::DIVIDED_MASK = 0x20;

const byte StreetSegmentItem::MULTI_DIGITISED_MASK = 0x40;

const byte StreetSegmentItem::CONTROLLED_ACCESS_MASK = 0x80;

uint32 
StreetSegmentItem::getMemoryUsage() const 
{
   uint32 totalSize = RouteableItem::getMemoryUsage()
      - sizeof(RouteableItem) + sizeof(StreetSegmentItem);
   
   return totalSize;
}

void 
StreetSegmentItem::save( DataBuffer& dataBuffer, const GenericMap& map ) const
{
   DEBUG_DB(mc2dbg << "      StreetSegmentItem::save()" << endl;)

   RouteableItem::save( dataBuffer, map );

   DEBUG_DB(mc2dbg << "         Saving attributes for this item" << endl;)

   dataBuffer.writeNextByte( m_condition );
   dataBuffer.writeNextByte( m_roadClass );
   byte tmpData = m_width & WIDTH_MASK;
   if (m_roundaboutish)
      tmpData |= ROUNDABOUTISH_MASK;

   dataBuffer.writeNextByte( tmpData );
   
   // Store the three flaggs in the streetNumberType-byte on disc
   tmpData = byte(m_streetNumberType) & STREETNUMBERTYPE_MASK;
   if (m_roundabout)
       tmpData |= ROUNDABOUT_MASK;
   if (m_ramp)
       tmpData |= RAMP_MASK;
   if (m_divided)
       tmpData |= DIVIDED_MASK;
   if (m_multiDigitised)
       tmpData |= MULTI_DIGITISED_MASK;
   if (m_controlledAccess)
      tmpData |= CONTROLLED_ACCESS_MASK;

   dataBuffer.writeNextByte(tmpData);

}

void
StreetSegmentItem::load( DataBuffer& dataBuffer, GenericMap& theMap )
{
   m_type = ItemTypes::streetSegmentItem;

   RouteableItem::load( dataBuffer, theMap );

   m_condition = dataBuffer.readNextByte();
   m_roadClass = dataBuffer.readNextByte();
   byte tmpData = dataBuffer.readNextByte();

   m_width = tmpData & WIDTH_MASK;
   m_roundaboutish = ( (tmpData & ROUNDABOUTISH_MASK) != 0);
       
   tmpData = dataBuffer.readNextByte();

   m_streetNumberType = 
      ItemTypes::streetNumberType(tmpData & STREETNUMBERTYPE_MASK);
   m_roundabout = ( (tmpData & ROUNDABOUT_MASK) != 0);
   m_ramp = ( (tmpData & RAMP_MASK) != 0);
   m_divided = ( (tmpData & DIVIDED_MASK) != 0);
   m_multiDigitised = ( (tmpData & MULTI_DIGITISED_MASK) != 0);
   m_controlledAccess = ( (tmpData & CONTROLLED_ACCESS_MASK) != 0);

   // The display class is not saved together with the item in the
   // current m3-format, instead it is stored in a separate table.
   m_displayClass = ItemTypes::nbrRoadDisplayClasses;
}

void 
StreetSegmentItem::setStreetNumberType(ItemTypes::streetNumberType x)
{
   m_streetNumberType = x;
}

void 
StreetSegmentItem::setDisplayClass( 
   ItemTypes::roadDisplayClass_t displayClass ) {

   m_displayClass = displayClass;
}
