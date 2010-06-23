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
#include "RouteableItem.h"

#include "MC2String.h"
#include "StringUtility.h"
#include "GenericMap.h"
#include "Node.h"

#include "FerryItem.h"
#include "BusRouteItem.h"
#include "StreetSegmentItem.h"

byte RouteableItem::getRoadClass() const {
   if ( getItemType() == ItemTypes::ferryItem )
      return item_cast< const FerryItem* >(this)->getRoadClass();
   if ( getItemType() == ItemTypes::streetSegmentItem )
      return item_cast< const StreetSegmentItem* >(this)->getRoadClass();
   if ( getItemType() == ItemTypes::busRouteItem ) 
      return item_cast< const BusRouteItem* >(this)->getRoadClass();

   mc2log << warn << here << " can not determine the right RouteableItem sub type for getRoadClass()" << endl;

   return ItemTypes::mainRoad;
}

void
RouteableItem::save( DataBuffer& dataBuffer, const GenericMap& map ) const
{
   Item::save( dataBuffer, map );
   m_node[0].save( dataBuffer, map );
   m_node[1].save( dataBuffer, map );
}


void
RouteableItem::load( DataBuffer& dataBuffer, GenericMap& theMap )
{
   Item::load( dataBuffer, theMap );
   m_node[0].setNodeID( m_localID & 0x7FFFFFFF );
   m_node[0].load( dataBuffer, theMap );
   m_node[1].setNodeID( m_localID | 0x80000000 );
   m_node[1].load( dataBuffer, theMap );

}

uint32 
RouteableItem::getMemoryUsage() const 
{
   uint32 totalSize = Item::getMemoryUsage()
      - sizeof(Item) + sizeof(RouteableItem);
   
   totalSize += 
      m_node[0].getMemoryUsage() + 
      m_node[1].getMemoryUsage();

   return totalSize;
}


