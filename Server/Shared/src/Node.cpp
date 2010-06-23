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

#include "Connection.h"
#include "DataBuffer.h"

#include "Node.h"
#include "GenericMap.h"
#include "AllocatorTemplate.h"
#include "BitUtility.h"

#define MAJOR_ROAD_BIT_NBR    7
#define ROAD_TOLL_BIT_NBR     6


Node::Node(uint32 nodeID) : m_entryConnections(NULL)
{
   mc2dbg8 << "Node created, nodeID=" << nodeID << endl;

   m_nodeID = nodeID;

   m_majorRoad = false;
   m_entryRestrictions = ItemTypes::noRestrictions;
   m_level = 0;
   m_nbrLanes = 1;

   m_roadToll = false;
   m_maximumWeight = 0x7; // Three bits
   m_maximumHeight = 0x3; // Two bits

   m_speedLimit = MAX_BYTE;
   
   m_junctionType = ItemTypes::normalCrossing;
}


void
Node::load( DataBuffer& dataBuffer, GenericMap& theMap ) 
{
   // The first byte...
   uint8 bitField = dataBuffer.readNextByte();
   m_majorRoad = BitUtility::getBit(bitField, MAJOR_ROAD_BIT_NBR);
   m_roadToll = BitUtility::getBit(bitField, ROAD_TOLL_BIT_NBR);
   m_entryRestrictions = ItemTypes::entryrestriction_t((bitField & 0x0C) >> 2);
   m_nbrLanes = ItemTypes::entryrestriction_t((bitField & 0x03));

   // The next three bytes
   m_level = (int8) dataBuffer.readNextByte();

   m_maximumWeight = dataBuffer.readNextByte();
   m_maximumHeight = dataBuffer.readNextByte();

   m_speedLimit = dataBuffer.readNextByte();
   m_junctionType = ItemTypes::junction_t(dataBuffer.readNextByte());
   m_nbrEntryConnections = dataBuffer.readNextShort();
   m_entryConnections = NULL;

   if ( m_nbrEntryConnections > 0 ) {
      m_entryConnections = (Connection *)(theMap.getConnectionAllocator().getNextObject());
      for ( uint32 i=0; i < m_nbrEntryConnections; ++i ) {
         if ( i != 0 ) {
            theMap.getConnectionAllocator().getNextObject();
         }
         m_entryConnections[ i ].load( dataBuffer );
      }
   }
}

void
Node::save( DataBuffer &dataBuffer, const GenericMap& map ) const
{
   uint8 bitField = 0;
   bitField = BitUtility::setBit(bitField, MAJOR_ROAD_BIT_NBR, m_majorRoad);
   bitField = BitUtility::setBit(bitField, ROAD_TOLL_BIT_NBR, m_roadToll);
   bitField |= ( uint32(m_entryRestrictions) << 2);
   bitField |= (m_nbrLanes);

   dataBuffer.writeNextByte(bitField);

   dataBuffer.writeNextByte(m_level);

   dataBuffer.writeNextByte(m_maximumWeight);
   dataBuffer.writeNextByte(m_maximumHeight);

   dataBuffer.writeNextByte(m_speedLimit);
   dataBuffer.writeNextByte(m_junctionType);
   dataBuffer.writeNextShort(m_nbrEntryConnections);

   for (uint32 i=0; i<m_nbrEntryConnections; i++) {
      m_entryConnections[ i ].save( dataBuffer );
   }
 
}

uint32 
Node::getMemoryUsage() const 
{
   uint32 conSize = 0;
   for (uint32 i=0; i<getNbrConnections(); ++i) {
      conSize += getEntryConnection(i)->getMemoryUsage();
   }
   return sizeof(*this) + conSize;
}


Connection*  
Node::getEntryConnectionFrom( uint32 fromNode ) const
{
   uint32 i = 0; 
   Connection* conn = NULL;
   while ( ( i < getNbrConnections() ) && ( conn == NULL ) ) { 
      if ( m_entryConnections[ i ].getConnectFromNode() == 
           fromNode ) {
         conn = &m_entryConnections[ i ];
      } else {
         ++i;
      }
   }
   return conn;
}
