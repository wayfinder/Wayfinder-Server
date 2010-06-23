/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSSTREETITEM_H
#define GMSSTREETITEM_H

#include "OldStreetItem.h"
#include "GMSMap.h"


/**
  *   GenerateMapServer street item. Street item
  *   with extra features needed when creating the map.
  *
  */
class GMSStreetItem : public OldStreetItem {
   public:
      /**
        *   Empty constructor that calls the empty constructor in 
        *   OldStreetItem.
        */
      GMSStreetItem() : OldStreetItem(MAX_UINT32) {
         m_nbrItemOffsetForPolygon = 0;
      }

      /**
        *   Constructor.
        */
      GMSStreetItem(uint32 id);
   
      /**
        *   The number of substeets in this street.
        */
      uint32 nbrOfSubStreets;

      /**
       *    Declare GMSItem as friend, to make it possible to
       *    generate streets from streetsegments.
       */
      friend class GMSMap;
      
   private:

      /**
       *    Sets the item for a polygon.
       *    @param polygon The polygon.
       *    @param itemID  The item id of the item.
       *    @return True if things went well, otherwise false.
       */
      bool setItemInPolygon(uint16 polygon, uint32 itemID);
      
      /**
       *    Add an offset to the m_itemOffsetForPolygon.
       *    @param offset  The offset to add.
       */
      void addItemOffsetForPolygon(uint16 offset);
      
      /**
       *    The number of elements in m_itemOffsetForPolygon
       *    during the generation of streets from streetsegments.
       */
      uint16 m_nbrItemOffsetForPolygon;
};

#endif
