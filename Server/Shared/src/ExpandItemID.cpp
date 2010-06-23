/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandItemID.h"

ExpandItemID::ExpandItemID()
      : m_itemID( 128, 256 ),   
        m_mapID ( 128, 256 ),
        m_groupID( 128, 256 ),
        m_lat   ( 128, 256 ),
        m_lon   ( 128, 256 ),
        m_coordinateOffset( 128, 256)
{
}


void ExpandItemID::addItem( uint32 mapID,
                            uint32 itemID )
{
   DEBUG8(cout << "Adding mapID " << mapID << " itemID " << hex
          << itemID << dec << endl);
   m_mapID.addLast(mapID);
   m_itemID.addLast(itemID);
   m_coordinateOffset.addLast(m_lat.getSize());
}

void ExpandItemID::addItem( uint32 mapID,
                            uint32 groupID,
                            uint32 itemID )
{
   addItem(mapID, itemID);
   DEBUG8( cout << "Adding addItem " << groupID << ":"
           << m_groupID.getSize() << endl);
   m_groupID.addLast(groupID);
}


void ExpandItemID::addItem( uint32 mapID,
                            uint32 itemID,
                            int32 lat,
                            int32 lon)
{
   addItem(mapID, itemID);
   DEBUG8(cout << "Adding latlong " << lat << "," << lon << endl);
   m_lat.addLast(lat);
   m_lon.addLast(lon);
}

void ExpandItemID::addItem( uint32 mapID,
                            uint32 groupID,
                            uint32 itemID,
                            int32 lat,
                            int32 lon )
{
   addItem(mapID, itemID, lat, lon);
   DEBUG8(cout << "Adding groupID " << groupID << endl);
   m_groupID.addLast(groupID);
}

void 
ExpandItemID::addSpeedLimit( byte speedLimit )
{
   m_speedLimit.addLast(speedLimit);
}

void 
ExpandItemID::addAttributes( byte attributes )
{
   m_attributes.addLast(attributes);
}

void
ExpandItemID::addCoordinate(int32 lat, int32 lon)
{
   m_lat.addLast(lat);
   m_lon.addLast(lon);
}
