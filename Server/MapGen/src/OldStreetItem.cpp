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

#include "OldStreetItem.h"
#include "DataBuffer.h"
#include "GfxData.h"

OldStreetItem::OldStreetItem(uint32 id)
    : OldGroupItem(ItemTypes::streetItem, id)
{
   mc2dbg4 << "OldStreetItem loaded" << endl;
   m_itemOffsetForPolygon = NULL;
}


OldStreetItem::~OldStreetItem()
{
   delete [] m_itemOffsetForPolygon;
}


bool
OldStreetItem::save(DataBuffer* dataBuffer) const
{
   DEBUG_DB(mc2dbg << "      OldStreetItem::save()" << endl);

   OldGroupItem::save(dataBuffer);
   if (m_gfxData != NULL) {
      for (uint16 i = 0; i < m_gfxData->getNbrPolygons(); i++) {
         dataBuffer->writeNextShort(m_itemOffsetForPolygon[i]);
      }
   }

   return (true);
}

char* OldStreetItem::toString()
{
   char tmpStr[4096];
   strcpy(tmpStr, OldGroupItem::toString());
   sprintf(itemAsString, "***** StreetItem\n%s", tmpStr);
   return itemAsString;
}

bool
OldStreetItem::createFromDataBuffer(DataBuffer *dataBuffer,
                                 OldGenericMap* theMap)
{
   m_type = ItemTypes::streetItem;
   OldGroupItem::createFromDataBuffer(dataBuffer, theMap);
   if (m_gfxData != NULL) {
      uint16 nbrPolygons = m_gfxData->getNbrPolygons();
      if (nbrPolygons > 0) {
         m_itemOffsetForPolygon = new uint16[nbrPolygons];
         for (uint16 i = 0; i < nbrPolygons; i++) {
            m_itemOffsetForPolygon[i] = dataBuffer->readNextShort();
         }
      } else {
         m_itemOffsetForPolygon = NULL;
      }
   } else {
      m_itemOffsetForPolygon = NULL;
   }
   return (true);
}

void
OldStreetItem::writeMifHeader(ofstream& mifFile)
{
   OldItem::writeGenericMifHeader(0, mifFile);
   //writeSpecificHeader(mif)
   mifFile << "DATA" << endl;
}

void
OldStreetItem::printMidMif(ofstream& midFile, ofstream& mifFile, OldItemNames* namePointer)
{
   OldItem::printMidMif(midFile, mifFile, namePointer);
   midFile << endl;
}

uint32 
OldStreetItem::getMemoryUsage() const 
{
   uint32 totalSize = OldGroupItem::getMemoryUsage()
      - sizeof(OldGroupItem) + sizeof(OldStreetItem);
   if ((m_gfxData != NULL) && (m_itemOffsetForPolygon != NULL)) {
      totalSize += m_gfxData->getNbrPolygons() * sizeof(uint16);
   }
   return (totalSize);
}

bool
OldStreetItem::updateAttributesFromItem(OldItem* otherItem, bool sameMap)
{
   bool retVal = false;
   mc2log << warn << "OldStreetItem::updateAttributesFromItem "
                  << "not implemented" << endl;
   // If updating e.g. name of street all street segment items in the
   // street should also be handled, can not be done in OldItem.

   return retVal;
}

uint32 
OldStreetItem::getItemInPolygon(uint16 polyIdx) const 
{
   if (polyIdx < getGfxData()->getNbrPolygons()) {
      return (getItemNumber(m_itemOffsetForPolygon[polyIdx]));
   } else {
      return (MAX_UINT32);
   }
}


