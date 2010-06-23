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

#include "DataBuffer.h"
#include "OldZipCodeItem.h"
#include "StringUtility.h"

OldZipCodeItem::OldZipCodeItem(uint32 id)
      : OldGroupItem(ItemTypes::zipCodeItem, id)
{
}


OldZipCodeItem::~OldZipCodeItem()
{
}

bool
OldZipCodeItem::save(DataBuffer* dataBuffer) const
{

   OldGroupItem::save(dataBuffer);

   // Empty strings not to break the mcm file format
   // These can be removed from the file-format when suitable...
   dataBuffer->writeNextString("");
   dataBuffer->writeNextString("");

   return (true);
}


uint32 
OldZipCodeItem::getMemoryUsage() const 
{
   return OldGroupItem::getMemoryUsage();
}

bool
OldZipCodeItem::createFromDataBuffer(DataBuffer *dataBuffer,
                                  OldGenericMap* theMap)
{
   DEBUG_DB(mc2dbg << "OldZipCodeItem is being created." << endl);

   m_type = ItemTypes::zipCodeItem;
   if (OldGroupItem::createFromDataBuffer(dataBuffer, theMap)) {
      // Empty strings not to break the mcm file format
      // These can be removed from the file-format when suitable...
      dataBuffer->readNextString();
      dataBuffer->readNextString();
      return true;
   } else {
      return false;
   }
}

bool
OldZipCodeItem::updateAttributesFromItem(OldItem* otherItem, bool sameMap)
{
   bool retVal = false;
   if ((otherItem == NULL) ||
       (otherItem->getItemType() != ItemTypes::zipCodeItem)) {
      return retVal;
   }

   // First general attributes such as names and gfxdata
   if (OldItem::updateAttributesFromItem(otherItem, sameMap))
      retVal = true;
   
   // No item specific attributes

   return retVal;
}

