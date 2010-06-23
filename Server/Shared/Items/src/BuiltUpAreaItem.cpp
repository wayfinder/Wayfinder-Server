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

#include "BuiltUpAreaItem.h"
#include "DataBuffer.h"
#include "GfxData.h"

void
BuiltUpAreaItem::save( DataBuffer& dataBuffer, const GenericMap& map ) const
{
   GroupItem::save(dataBuffer, map );
   dataBuffer.writeNextLong(m_nbrOfInhabitants);
}

void
BuiltUpAreaItem::load( DataBuffer& dataBuffer, GenericMap& theMap )
{
   m_type = ItemTypes::builtUpAreaItem;
   GroupItem::load( dataBuffer, theMap );
   m_nbrOfInhabitants = dataBuffer.readNextLong();

   // The display class is not saved together with the item in the
   // current m3-format, instead it is stored in a separate table.
   m_displayClass = ItemTypes::nbrAreaFeatureDrawDisplayClasses;

}

ItemTypes::city_t 
BuiltUpAreaItem::getBuiltUpAreaType() 
{
   // No gfxdata -> village
   if ( m_gfxData == NULL )
      return ItemTypes::VILLAGE_CITY;
   
   const uint64 bigCityMinArea = uint64(1000000)*uint64(1000000);
   const uint64 mediumCityMinArea = uint64(600000)*uint64(600000);
   const uint64 smallCityMinArea = uint64(400000)*uint64(400000);
   
   uint64 area = m_gfxData->getBBoxArea_mc2();
   if (area > bigCityMinArea)
      return ItemTypes::BIG_CITY;
   else if (area > mediumCityMinArea)
      return ItemTypes::MEDIUM_CITY;
   else if (area > smallCityMinArea)
      return ItemTypes::SMALL_CITY;
   else
      return ItemTypes::VILLAGE_CITY;
}

void
BuiltUpAreaItem::setDisplayClass( 
   ItemTypes::areaFeatureDrawDisplayClass_t displayClass ) {
   m_displayClass = displayClass;
}

