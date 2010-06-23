/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AddDisturbanceData.h"
#include "PacketHelpers.h"
#include "Packet.h"

const AddDisturbanceRequestData::ExtentType AddDisturbanceRequestData::MAX_EXTENT = MAX_UINT8;

int
AddDisturbanceRequestData::getSizeInPacket() const
{
   return ( 4 * 13 + 
            m_nodes.size() * 4 * 5 +
            m_firstLocation.length() + 1 + 
            m_secondLocation.length() + 1 +
            m_situationReference.length() + 1 +
            m_text.length() + 1 );
}


int AddDisturbanceRequestData::save(Packet* packet, int& pos) const
{
   packet->updateSize( getSizeInPacket(), packet->getBufSize() );
   const int startPos = pos;

   packet->incWriteLong(pos, m_nodes.size());  // Nbr of disturbances
   packet->incWriteLong(pos, m_disturbanceID);
   packet->incWriteLong(pos, m_type);
   packet->incWriteLong(pos, m_phrase);
   packet->incWriteLong(pos, m_eventCode);
   packet->incWriteLong(pos, m_startTime);
   packet->incWriteLong(pos, m_endTime);
   packet->incWriteLong(pos, m_creationTime);
   packet->incWriteLong(pos, m_severity);
   packet->incWriteLong(pos, m_direction);
   packet->incWriteLong(pos, m_extent);
   packet->incWriteLong(pos, m_costFactor);
   packet->incWriteLong(pos, m_queueLength);

   for(vector<DisturbanceNode>::const_iterator it = m_nodes.begin(); 
        it != m_nodes.end(); ++it ) {
      packet->incWriteLong(pos, it->nodeID);
      packet->incWriteLong(pos, it->lat);
      packet->incWriteLong(pos, it->lon);
      packet->incWriteLong(pos, it->angle);
      packet->incWriteLong(pos, it->routeIndex);
   }

   packet->incWriteString(pos, m_firstLocation.c_str());
   packet->incWriteString(pos, m_secondLocation.c_str());
   packet->incWriteString(pos, m_situationReference.c_str());
   packet->incWriteString(pos, m_text.c_str());

   MC2_ASSERT( ( pos - startPos ) == getSizeInPacket() );
   
   packet->setLength( pos );

   return getSizeInPacket();
}

int AddDisturbanceRequestData::load(const Packet* packet, int& pos)
{
   const uint32 numNodes = packet->incReadLong(pos);  // Nbr of disturbances

   m_disturbanceID = packet->incReadLong(pos);
   m_type          = 
      TrafficDataTypes::disturbanceType( packet->incReadLong(pos) );
   m_phrase        = TrafficDataTypes::phrase( packet->incReadLong(pos) );
   m_eventCode     = packet->incReadLong(pos);
   m_startTime     = packet->incReadLong(pos);
   m_endTime       = packet->incReadLong(pos);
   m_creationTime  = packet->incReadLong(pos);
   m_severity      = TrafficDataTypes::severity( packet->incReadLong(pos) );
   m_direction     = TrafficDataTypes::direction( packet->incReadLong(pos) );
   m_extent        = packet->incReadLong(pos);
   m_costFactor    = packet->incReadLong(pos);
   m_queueLength   = packet->incReadLong(pos);

   for( uint32 i = 0; i < numNodes; ++i){
      uint32 nodeID     = packet->incReadLong(pos);
       int32 lat        = packet->incReadLong(pos);
       int32 lon        = packet->incReadLong(pos);
      uint32 angle      = packet->incReadLong(pos);
      uint32 routeIndex = packet->incReadLong(pos);
      m_nodes.push_back(DisturbanceNode(nodeID, lat, lon, angle, routeIndex));
   }

   m_firstLocation      = packet->incReadString(pos);
   m_secondLocation     = packet->incReadString(pos);
   m_situationReference = packet->incReadString(pos);
   m_text               = packet->incReadString(pos);

   return getSizeInPacket();
}
