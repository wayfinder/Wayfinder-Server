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

#include "OldGroupItem.h"
#include "DataBuffer.h"

OldGroupItem::OldGroupItem(ItemTypes::itemType type, uint32 id) 
   : OldItem (type, id), m_itemsInGroup(NULL), m_nbrItemsInGroup(0)
{
   
}


OldGroupItem::~OldGroupItem()
{
   delete [] m_itemsInGroup;
}

bool
OldGroupItem::save(DataBuffer* dataBuffer) const
{
   OldItem::save(dataBuffer);

   dataBuffer->writeNextLong(m_nbrItemsInGroup);
   DEBUG_DB(cerr << "nbrItemsInGroup =" << m_nbrItemsInGroup << endl);
   for (uint32 i=0; i<m_nbrItemsInGroup; i++) {
      dataBuffer->writeNextLong(m_itemsInGroup[i]);
   }

   return (true);
}


bool
OldGroupItem::createFromDataBuffer(DataBuffer *dataBuffer, 
                                OldGenericMap* theMap)
{
   if (OldItem::createFromDataBuffer(dataBuffer, theMap)) {
      m_nbrItemsInGroup = dataBuffer->readNextLong();

      DEBUG_DB(cerr << "nbrItemsInGroup = " << m_nbrItemsInGroup << endl);
      if (m_nbrItemsInGroup > 0) {
         m_itemsInGroup = new uint32[m_nbrItemsInGroup];
         for (uint32 i=0; i<m_nbrItemsInGroup; i++) {
            m_itemsInGroup[i] = dataBuffer->readNextLong();
         }
      } else {
         m_itemsInGroup = NULL;
      }
      return true;
   } else {
      mc2log << fatal << here << "OldItem::createFromDataBuffer returned false"
             << endl;
      return false;
   }
}


bool 
OldGroupItem::removeItemWithID(uint32 id) 
{
   uint32 pos = 0;
   while ( (pos < getNbrItemsInGroup()) && (m_itemsInGroup[pos] != id)) {
      ++pos;
   }
   if (pos < getNbrItemsInGroup()) {
      return (removeItemNumber(pos));
   }
   return false;
}


char* OldGroupItem::toString()
{
   // Copy the OldItem to tmpStr
   strcpy(itemAsString, OldItem::toString());

   // Print this item into tmpStr
   char tmpStr[4096];
   uint32 writtenChars = sprintf(tmpStr, 
                                 "nbrItemsInGroup=%u\nitemsInGroup:",
                                 getNbrItemsInGroup());
   for (uint32 i=0; i<getNbrItemsInGroup(); ++i) {
      writtenChars += sprintf(tmpStr + writtenChars, 
                              "   %u. %u\n",
                              i, 
                              m_itemsInGroup[i]);
      if (writtenChars > 3000) {
         sprintf(tmpStr + writtenChars, "and so on...");
         break;
      }
   }

   // Concatenate the global one and the temporary string
   strcat(itemAsString, tmpStr);
   return itemAsString;
}

uint32 
OldGroupItem::getMemoryUsage() const {
   return OldItem::getMemoryUsage() + m_nbrItemsInGroup * sizeof(uint32);
}
      
bool 
OldGroupItem::containsItem( uint32 itemID ) const
{
   uint32 i = 0;
   bool found = false;
   while ( ( i < getNbrItemsInGroup() ) && ( ! found ) ) {
      if ( getItemNumber( i ) == itemID ) {
         found = true;
      } else {
         i++;
      }
   }

   return found;
}

