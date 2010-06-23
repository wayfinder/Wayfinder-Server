/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSITEM_H
#define GMSITEM_H

#include "DataBuffer.h"
#include "MapData.h"

class GMSMap;
class OldItem;

/**
 *
 */
class GMSItem {
   public:
      /**
       *    Input map types
       */
      enum map_t {GDF = 0, INVALID = 2, MIDMIF = 3};

      /**
       *    Constructor.
       */
      GMSItem(map_t mapType);

      /**
       *    Destructor.
       */
      virtual ~GMSItem();
   
      /**
       *    Create a new item with the correct dynamic type from a
       *    databuffer.
       *    @param   dataBuffer  The buffer that contains data for
       *                         the item.
       *    @return  The new item.
       */
      static OldItem* createNewItem(DataBuffer* dataBuffer, GMSMap* map);

      /**
       *    @return MapData
       */
      inline MapData* getMapData();

      /**
       *    Get left settlement for this item. Checks if any such attribute
       *    exists in the mapData.
       */
      uint32 getLeftSettlement();
      
      /**
       *    Set left settlement for this item. Checks if any such attribute
       *    exists in the mapData.
       */
      void setLeftSettlement(uint32 settlement);
      
      /**
       *    Get right settlement for this item. Checks if any such attribute
       *    exists in the mapData.
       */
      uint32 getRightSettlement();
      
      /**
       *    Set right settlement for this item. Checks if any such attribute
       *    exists in the mapData.
       */
      void setRightSettlement(uint32 settlement);

      /**
       *    Get settlement order for this item. Checks if any such attribute
       *    exists in the mapData.
       */
      uint32 getSettlementOrder();
      
      /**
       *    Set settlement order for this item. Checks if any such attribute
       *    exists in the mapData.
       */
      void setSettlementOrder(uint32 settlement);

      /**
       *    Get temp index area order for this item.
       *    Checks if any such attribute exists in the mapData.
       */
      uint32 getTempIndexAreaOrder();
      
      /**
       *    Set temp index area order for this item.
       *    Checks if any such attribute exists in the mapData.
       */
      void setTempIndexAreaOrder(uint32 order);

      /**
       *    Get temp road display class for this item, as defined in 
       *    ItemTypes::roadDisplayClass_t.
       *    Checks if any such attribute exists in the mapData.
       *    If no road display class exists, returns MAX_UINT32.
       */
      uint32 getTempRoadDisplayClass();
      

   private:
      /**
       *    MapData, used to store temporary data when creating
       *    items from the raw map data files
       */
      MapData* m_mapData;

      /**
       *    Initiate the mapData member variables in this object.
       */
      void initMapType(map_t mapType);
      
};

inline MapData*
GMSItem::getMapData()
{
   return m_mapData;
}



#endif
