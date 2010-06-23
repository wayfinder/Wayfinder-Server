/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MIDMIFDATA_H
#define MIDMIFDATA_H

//#include "config.h"
#include "MapData.h"

/**
  *   Used when creating items from mid mif.
  */
class MidMifData : public MapData {
   public:
      /**
        *   Creates an empty MidMIfData.
        */
      MidMifData();

      /**
        *   Destroys the MidMIfData.
        */
      virtual ~MidMifData();
      
      ///   Get left settlement.
      inline uint32 getLeftSettlement();

      /// Set left settlement.
      inline void setLeftSettlement(uint32 settlement);

      /// Get right settlement.
      inline uint32 getRightSettlement();

      // Set right settlement.
      inline void setRightSettlement(uint32 settlement);
      
      /// Get settlement order.
      inline uint32 getSettlementOrder();
      
      /// Set settlement order.
      inline void setSettlementOrder(uint32 order);
      
      /// Get index area order.
      inline uint32 getTempIndexAreaOrder();
      
      /// Set settlement order.
      inline void setTempIndexAreaOrder(uint32 order);
      
      /// Get road display class.
      inline uint32 getTempRoadDisplayClass();
      
      /// Set road display class.
      inline void setTempRoadDisplayClass(uint32 dispClass);
      
   private:
      /// Left settlement of the item.
      uint32 m_leftSettlement;
      
      /// Right settlement of the item.
      uint32 m_rightSettlement;

      /// The order of settlements
      uint32 m_settlementOrder;
      
      /**
        * The index area order of the item. Deafult is MAX_UINT32
        * which means no index area order set.
        * The attribute is called tmp, not to confuse it with the index 
        * area order that is stored in the map.
        */
      uint32 m_tmpIndexAreaOrder;
      
      /**
        * Road display class when creating form midmif, 
        * will be permanently stored in the map later.
        */
      uint32 m_tmpRoadDisplayClass;
};

inline uint32
MidMifData::getLeftSettlement()
{
   return m_leftSettlement;
}

inline void
MidMifData::setLeftSettlement(uint32 settlement)
{
   m_leftSettlement = settlement;
}

inline uint32
MidMifData::getRightSettlement()
{
   return m_rightSettlement;
}

inline void
MidMifData::setRightSettlement(uint32 settlement)
{
   m_rightSettlement = settlement;
}

inline uint32
MidMifData::getSettlementOrder()
{
   return m_settlementOrder;
}

inline void
MidMifData::setSettlementOrder(uint32 order)
{
   m_settlementOrder = order;
}

inline uint32
MidMifData::getTempIndexAreaOrder()
{
   return m_tmpIndexAreaOrder;
}

inline void
MidMifData::setTempIndexAreaOrder(uint32 order)
{
   m_tmpIndexAreaOrder = order;
}

inline uint32
MidMifData::getTempRoadDisplayClass()
{
   return m_tmpRoadDisplayClass;
}

inline void
MidMifData::setTempRoadDisplayClass(uint32 dispClass)
{
   m_tmpRoadDisplayClass = dispClass;
}

#endif

