/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "OldForestItem.h"

OldForestItem::OldForestItem(uint32 id) 
   : OldItem(ItemTypes::forestItem, id)
{

}

OldForestItem::~OldForestItem()
{

}


bool 
OldForestItem::save(DataBuffer* dataBuffer) const
{
   DEBUG_DB(cerr << "      OldForestItem::save()" << endl;)

   return (OldItem::save(dataBuffer));
}

char* 
OldForestItem::toString()
{
   char tmpStr[1024];
   strcpy(tmpStr, OldItem::toString());
   sprintf(itemAsString, "***** ForestItem\n%s", tmpStr);
   return itemAsString;
}

bool
OldForestItem::createFromDataBuffer(DataBuffer *dataBuffer, 
                                 OldGenericMap* theMap)
{
   m_type = ItemTypes::forestItem;
   return  (OldItem::createFromDataBuffer(dataBuffer, theMap));
}


void
OldForestItem::writeMifHeader(ofstream& mifFile)
{
   OldItem::writeGenericMifHeader(0, mifFile);
   //writeSpecificHeader(mif)
   mifFile << "DATA" << endl;
}
void
OldForestItem::printMidMif(
      ofstream& midFile, ofstream& mifFile, OldItemNames* namePointer)
{
   OldItem::printMidMif(midFile, mifFile, namePointer);
   midFile << endl;
}

bool
OldForestItem::updateAttributesFromItem(OldItem* otherItem, bool sameMap)
{
   bool retVal = false;
   if ((otherItem == NULL) ||
       (otherItem->getItemType() != ItemTypes::forestItem)) {
      return retVal;
   }

   // First general attributes such as names and gfxdata
   if (OldItem::updateAttributesFromItem(otherItem, sameMap))
      retVal = true;
   
   // No item specific attributes

   return retVal;
}

