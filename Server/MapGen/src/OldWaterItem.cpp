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
#include "OldWaterItem.h"
#include "DataBuffer.h"


OldWaterItem::OldWaterItem(uint32 id) 
   : OldItem(ItemTypes::waterItem, id)
{
   m_waterType = ItemTypes::lake;
}  


OldWaterItem::~OldWaterItem()
{

}


bool 
OldWaterItem::save(DataBuffer* dataBuffer) const
{
   DEBUG_DB(cerr << "      OldWaterItem::save()" << endl;)

   OldItem::save(dataBuffer);
   
   dataBuffer->writeNextByte(m_waterType);
   
   return (true);
}

char* OldWaterItem::toString()
{
   char tmpStr[1024];
   strcpy(tmpStr, OldItem::toString());
   sprintf(itemAsString, "***** WaterItem\n%s", tmpStr);
   return itemAsString;
}

bool
OldWaterItem::createFromDataBuffer(DataBuffer *dataBuffer,
                                OldGenericMap* theMap)
{
   m_type = ItemTypes::waterItem;
   if (OldItem::createFromDataBuffer(dataBuffer, theMap)) {
      m_waterType = dataBuffer->readNextByte();
      return (true);
   } else {
      mc2log << warn << "Failed to create OldWaterItem from databuffer" << endl;
      m_waterType = ItemTypes::lake;
      return (false);
   }
}

void
OldWaterItem::writeMifHeader(ofstream& mifFile)
{
   OldItem::writeGenericMifHeader(2, mifFile);
   //writeSpecificHeader(mif)
   mifFile << "  WATER_TYPE char(25)\r\n"
           << "  MAP_ID decimal(10,0)\r\n"
           << "DATA" << endl;
}
void
OldWaterItem::printMidMif(ofstream& midFile, ofstream& mifFile, 
                       OldItemNames* namePointer)
{
   OldItem::printMidMif(midFile, mifFile, namePointer);
   //byte type = getWaterType();
   midFile << ",\"" ;
   switch (m_waterType){
      case ItemTypes::ocean : {
         midFile << "ocean";
      } break;
      case ItemTypes::lake : {
         midFile << "lake";
      } break;
      case ItemTypes::river : {
         midFile << "river";
      } break;
      case ItemTypes::canal : {
         midFile << "canal";
      } break;
      case ItemTypes::harbour : {
         midFile << "harbour";
      } break;
      case ItemTypes::otherWaterElement : {
         midFile << "otherWaterElement";
      } break;
      case ItemTypes::unknownWaterElement : {
         midFile << "unknownWaterElement";
      } break;
      default: {
         mc2log << error << "Unhandled waterType " << int(m_waterType)
                << endl;
         MC2_ASSERT(false);
      } break;
   }
   midFile << "\"";
//           << endl; //OldGenericMap prints mapid 
}

bool
OldWaterItem::updateAttributesFromItem(OldItem* otherItem, bool sameMap)
{
   bool retVal = false;
   if ((otherItem == NULL) ||
       (otherItem->getItemType() != ItemTypes::waterItem)) {
      return retVal;
   }

   // First general attributes such as names and gfxdata
   if (OldItem::updateAttributesFromItem(otherItem, sameMap))
      retVal = true;
   
   // Then item specific attributes
   OldWaterItem* otherWater = (OldWaterItem*) otherItem;
   if (otherWater != NULL) {
      if (m_waterType != otherWater->getWaterType()) {
         m_waterType = otherWater->getWaterType();
         retVal = true;
      }
   }

   return retVal;
}

