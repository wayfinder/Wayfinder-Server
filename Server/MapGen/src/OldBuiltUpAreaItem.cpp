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

#include "OldBuiltUpAreaItem.h"
#include "DataBuffer.h"
#include "GfxData.h"

OldBuiltUpAreaItem::OldBuiltUpAreaItem(uint32 id)
   : OldGroupItem(ItemTypes::builtUpAreaItem, id)
{

}

OldBuiltUpAreaItem::~OldBuiltUpAreaItem()
{
   MINFO("~BuildUpAreaItem()");
}

bool
OldBuiltUpAreaItem::save(DataBuffer *dataBuffer) const
{
   DEBUG_DB( mc2dbg << "      OldBuiltUpAreaItem::save()" << endl );

   bool retVal = OldGroupItem::save(dataBuffer);
   MC2_ASSERT(retVal);
   if (retVal) {
      dataBuffer->writeNextLong(m_nbrOfInhabitants);
      DEBUG_DB(mc2dbg << "nbrOfInhabitants=" << m_nbrOfInhabitants << endl;);
   } else {
      mc2log << fatal << here << " OldGroupItem::save returned false" << endl;
   }
   return retVal;
}

bool
OldBuiltUpAreaItem::createFromDataBuffer(DataBuffer* dataBuffer, 
                                      OldGenericMap* theMap)
{
   m_type = ItemTypes::builtUpAreaItem;
   bool retVal = OldGroupItem::createFromDataBuffer(dataBuffer, theMap);
   MC2_ASSERT(retVal);
   if (retVal) {
      m_nbrOfInhabitants = dataBuffer->readNextLong();
      DEBUG_DB(mc2dbg << "nbrOfInhabitants=" << m_nbrOfInhabitants << endl;);
   } else {
      mc2log << fatal << here << " OldGroupItem::createFromDataBuffer "
             << "returned false" << endl;
   }
   return retVal;
}

char* OldBuiltUpAreaItem::toString()
{
   char tmpStr[1024];
   strcpy(tmpStr, OldItem::toString());
   sprintf(itemAsString,   "***** BuiltUpAreaItem\n"
                           "%s",
                           tmpStr);
   return itemAsString;
}


void
OldBuiltUpAreaItem::writeMifHeader(ofstream& mifFile)
{
   OldItem::writeGenericMifHeader(0, mifFile);
   //writeSpecificHeader(mif)
   mifFile << "DATA" << endl;
}
void
OldBuiltUpAreaItem::printMidMif(ofstream& midFile, ofstream& mifFile, OldItemNames* namePointer)
{
   OldItem::printMidMif(midFile, mifFile, namePointer);
   midFile << endl;
}

bool
OldBuiltUpAreaItem::updateAttributesFromItem(OldItem* otherItem, bool sameMap)
{
   bool retVal = false;
   mc2dbg4 << "OldBuiltUpAreaItem::updateAttributesFromItem" << endl;
   if (otherItem == NULL) {
      return retVal;
   }

   // First general attributes such as names and gfxdata
   if (OldItem::updateAttributesFromItem(otherItem, sameMap))
      retVal = true;
   
   // Then item specific attributes
   OldBuiltUpAreaItem* otherBua = (OldBuiltUpAreaItem*) otherItem;
   if (otherBua != NULL) {
      if (m_nbrOfInhabitants != otherBua->getNbrInhabitants()) {
         m_nbrOfInhabitants = otherBua->getNbrInhabitants();
         retVal = true;
      }
   }

   return retVal;
}

ItemTypes::city_t 
OldBuiltUpAreaItem::getBuiltUpAreaType() 
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


