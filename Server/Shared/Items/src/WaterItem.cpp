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
#include "WaterItem.h"
#include "DataBuffer.h"


WaterItem::WaterItem(uint32 id) 
   : Item(ItemTypes::waterItem, id)
{
   m_waterType = ItemTypes::lake;
}  


WaterItem::~WaterItem()
{

}


void 
WaterItem::save( DataBuffer& dataBuffer, const GenericMap& map ) const
{
   Item::save( dataBuffer, map );
   dataBuffer.writeNextByte( m_waterType );
}

void
WaterItem::load( DataBuffer& dataBuffer, GenericMap& theMap )
{
   m_type = ItemTypes::waterItem;
   Item::load(dataBuffer, theMap);
   m_waterType = dataBuffer.readNextByte();

   // The display class is not saved together with the item in the
   // current m3-format, instead it is stored in a separate table.
   m_displayClass = ItemTypes::nbrAreaFeatureDrawDisplayClasses;
}

void
WaterItem::setDisplayClass( 
   ItemTypes::areaFeatureDrawDisplayClass_t displayClass ) {
   m_displayClass = displayClass;
}
