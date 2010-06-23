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
#include "Vector.h"
#include "Node.h"
#include "ItemTypes.h"
#include "GenericMap.h"
#include "AllocatorTemplate.h"
#include "Math.h"

const float64 Connection::NORMAL_SPEED_KMH = 50;
const float64 Connection::NORMAL_SPEED_MS = Connection::NORMAL_SPEED_KMH *
                                            Math::KMH_TO_MS;

const float64 Connection::PENALTY_ROAD_CLASS_0 = 1.0;
const float64 Connection::PENALTY_ROAD_CLASS_1 = 1.05;
const float64 Connection::PENALTY_ROAD_CLASS_2 = 1.05;
const float64 Connection::PENALTY_ROAD_CLASS_3 = 1.3;
const float64 Connection::PENALTY_ROAD_CLASS_4 = 1.5;
const float64 Connection::PENALTY_ROAD_CLASS_4_30KMH = 1.7;
const float64 Connection::PENALTY_RAMP = 0.30;

// Penalties (in seconds) for turning...
// The penalties for uturns are maybe a bit high but uturns are a bit
// dangerous and should be avoided.
// U-turns should be as expensive as driving around a block. 3 left and
// 1 right turn, the driving distance is simulated by quadrupling that,
// also as u-turns are not always possible in real life the cost should be
// high.
const uint32 Connection::STANDSTILLTIME_LEFTTURN_MAJOR_ROAD  = 20;
const uint32 Connection::STANDSTILLTIME_LEFTTURN_MINOR_ROAD  = 10;
const uint32 Connection::STANDSTILLTIME_RIGHTTURN_MAJOR_ROAD = 15;
const uint32 Connection::STANDSTILLTIME_RIGHTTURN_MINOR_ROAD = 10;
const uint32 Connection::STANDSTILLTIME_ENTER_ROUNDABOUT     = 15;
const uint32 Connection::STANDSTILLTIME_ENTER_FERRY          = 25 * 60;
const uint32 Connection::STANDSTILLTIME_EXIT_FERRY           = 3 * 60;
const uint32 Connection::STANDSTILLTIME_CHANGE_FERRY         = 3 * 60;
const uint32 Connection::STANDSTILLTIME_U_TURN_MAJOR_ROAD    = /*60;*/
   4 * (3 * Connection::STANDSTILLTIME_LEFTTURN_MAJOR_ROAD +
        1 * Connection::STANDSTILLTIME_RIGHTTURN_MAJOR_ROAD);
const uint32 Connection::STANDSTILLTIME_U_TURN_MINOR_ROAD    = /*30;*/
   4 * (3 * Connection::STANDSTILLTIME_LEFTTURN_MINOR_ROAD +
        1 * Connection::STANDSTILLTIME_RIGHTTURN_MINOR_ROAD);


uint32 Connection::tollRoadTimeDefaultPenalty_s     = 3600;
uint32 Connection::tollRoadDistanceDefaultPenalty_m = 10000;
float  Connection::highwayDefaultPenaltyFactor      = 3.0;

Connection::Connection(uint32 fromID)
{
   m_connectFromNodeIndex = fromID;
   m_turnDirection = ItemTypes::UNDEFINED;
   m_crossingKind = ItemTypes::UNDEFINED_CROSSING;
   m_exitCount = 0;
   m_vehicleRestrictionIdx = MAX_UINT8;
}

void
Connection::load( DataBuffer &dataBuffer )
{
   // Read the ID of the node this connection leads from.
   m_connectFromNodeIndex = dataBuffer.readNextLong();

   // Read vehicles that are allowed traversing this connection
   m_vehicleRestrictionIdx = dataBuffer.readNextByte();
   // Read turn-direction, crossing kind, exit count and nbr signposts.
   m_turnDirection = ItemTypes::turndirection_t( dataBuffer.readNextByte() );
   m_crossingKind = ItemTypes::crossingkind_t( dataBuffer.readNextByte() );
   m_exitCount =  dataBuffer.readNextByte();

   dataBuffer.alignToLong();
}

void
Connection::save( DataBuffer &dataBuffer ) const
{

   dataBuffer.writeNextLong(m_connectFromNodeIndex);

   dataBuffer.writeNextByte(m_vehicleRestrictionIdx);
   dataBuffer.writeNextByte(m_turnDirection);
   dataBuffer.writeNextByte(m_crossingKind);
   dataBuffer.writeNextByte(m_exitCount);

   dataBuffer.alignToLong();

}

Connection* 
Connection::createNewConnection( DataBuffer& dataBuffer, 
                                 GenericMap& theMap ) 
{
   Connection* con = theMap.getConnectionAllocator().getNextObject();

   con->load( dataBuffer );
   return con;
}

    

float64
Connection::getPenaltyFactor(byte roadClass, 
                             bool isRamp,
                             byte speedLimit)
{
   
   // Set penalty factor based on roadclass.
   float64 penaltyFactor;
   switch (roadClass) {
      case (0) :
            penaltyFactor = PENALTY_ROAD_CLASS_0;
         break;
      case (1) :
            penaltyFactor = PENALTY_ROAD_CLASS_1;
         break;
      case (2) :
            penaltyFactor = PENALTY_ROAD_CLASS_2;
         break;
      case (3) :
            penaltyFactor = PENALTY_ROAD_CLASS_3;
         break;
      default :
            
            if (speedLimit <= 30) {
               // More costly to drive on a 30km/h road
               penaltyFactor = PENALTY_ROAD_CLASS_4_30KMH; 
            } else {
               penaltyFactor = PENALTY_ROAD_CLASS_4;
            }
         break;
   }

   // Multiply the penalty factor with U-turn penalty in case
   // of u-turn.
   if (isRamp) {
      penaltyFactor += PENALTY_RAMP;
   }
   
   return (penaltyFactor);
}

void 
Connection::setExitCount( byte exitCount )
{
   m_exitCount = exitCount;
}

void 
Connection::setFromNode( uint32 fromNodeID )
{
   m_connectFromNodeIndex = fromNodeID;
}

uint32 
Connection::getMemoryUsage() const 
{
   uint32 sizeOfSignPosts = 0;

   return (sizeof(Connection) + sizeOfSignPosts);
}

void 
Connection::setCrossingKind(ItemTypes::crossingkind_t x) 
{
   m_crossingKind = x;
}

