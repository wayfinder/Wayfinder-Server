/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSStreetItem.h"
#include "GMSStreetSegmentItem.h"

GMSStreetItem::GMSStreetItem(uint32 id)
   : OldStreetItem(id)
{
   init(ItemTypes::streetItem);
   m_nbrItemOffsetForPolygon = 0;
}


void 
GMSStreetItem::addItemOffsetForPolygon(uint16 offset)
{
   // Allocate the new vector.
   uint16* tmpOffset = new uint16[m_nbrItemOffsetForPolygon + 1];
   
   // Copy the old offsets.
   for (uint16 i = 0; i < m_nbrItemOffsetForPolygon; i++) {
      tmpOffset[i] = m_itemOffsetForPolygon[i];
   }
   
   // And the new one.
   tmpOffset[m_nbrItemOffsetForPolygon++] = offset;

   // Delete the old vector and use the new one instead.
   delete m_itemOffsetForPolygon;
   m_itemOffsetForPolygon = tmpOffset;
}

bool
GMSStreetItem::setItemInPolygon(uint16 polygon, uint32 itemID) 
{
   // Find the offset for the itemID
   uint32 i = 0;
   bool done = false;
   while ( (i < getNbrItemsInGroup()) && (! done) ) {
      if (getItemNumber(i) != itemID) {
         i++;
      } else {
         done = true;
      }
   }

   if (!done) {
      // Couldn't find the ssi id.
      return (false);
   }
   
   if (polygon == m_nbrItemOffsetForPolygon) {
      // Add a new item offset.
      addItemOffsetForPolygon(i);
   } else if (polygon < m_nbrItemOffsetForPolygon) {
      // Replace a an item offset
      m_itemOffsetForPolygon[polygon] = i;
   } else {
      // polygon out of range.
      return (false);
   }

   return (true);
}

