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

#include "DataBuffer.h"
#include "Vector.h"
#include "OldConnection.h"
#include "OldNode.h"
#include "ItemTypes.h"
#include "OldGenericMap.h"
#include "AllocatorTemplate.h"
#include "SignPostTable.h"
#include "Math.h"

const float64 OldConnection::NORMAL_SPEED_KMH = 50;
const float64 OldConnection::NORMAL_SPEED_MS = OldConnection::NORMAL_SPEED_KMH *
                                            Math::KMH_TO_MS;

const float64 OldConnection::PENALTY_ROAD_CLASS_0 = 1.0;
const float64 OldConnection::PENALTY_ROAD_CLASS_1 = 1.05;
const float64 OldConnection::PENALTY_ROAD_CLASS_2 = 1.05;
const float64 OldConnection::PENALTY_ROAD_CLASS_3 = 1.3;
const float64 OldConnection::PENALTY_ROAD_CLASS_4 = 1.5;
const float64 OldConnection::PENALTY_ROAD_CLASS_4_30KMH = 1.7;
const float64 OldConnection::PENALTY_RAMP = 0.30;

// Penalties (in seconds) for turning...
// The penalties for uturns are maybe a bit high but uturns are a bit
// dangerous and should be avoided.
const uint32 OldConnection::STANDSTILLTIME_U_TURN_MAJOR_ROAD    = 60;
const uint32 OldConnection::STANDSTILLTIME_U_TURN_MINOR_ROAD    = 30;
const uint32 OldConnection::STANDSTILLTIME_LEFTTURN_MAJOR_ROAD  = 20;
const uint32 OldConnection::STANDSTILLTIME_LEFTTURN_MINOR_ROAD  = 10;
const uint32 OldConnection::STANDSTILLTIME_RIGHTTURN_MAJOR_ROAD = 15;
const uint32 OldConnection::STANDSTILLTIME_RIGHTTURN_MINOR_ROAD = 10;
const uint32 OldConnection::STANDSTILLTIME_ENTER_ROUNDABOUT     = 15;
const uint32 OldConnection::STANDSTILLTIME_ENTER_FERRY          = 25 * 60;
const uint32 OldConnection::STANDSTILLTIME_EXIT_FERRY           = 3 * 60;
const uint32 OldConnection::STANDSTILLTIME_CHANGE_FERRY         = 3 * 60;

uint32 OldConnection::tollRoadTimeDefaultPenalty_s     = 3600;
uint32 OldConnection::tollRoadDistanceDefaultPenalty_m = 10000;
float  OldConnection::highwayDefaultPenaltyFactor      = 3.0;

OldConnection::OldConnection(uint32 fromID)
{
   m_connectFromNodeIndex = fromID;
   m_turnDirection = ItemTypes::UNDEFINED;
   m_crossingKind = ItemTypes::UNDEFINED_CROSSING;
   m_exitCount =  0;
   m_vehicleRestriction = MAX_UINT32;
   m_restrictionsExplicitlySet = false;
}


OldConnection::OldConnection(DataBuffer *dataBuffer)
{
   createFromDataBuffer(dataBuffer);
}


void
OldConnection::createFromDataBuffer(DataBuffer *dataBuffer)
{
   // Read the ID of the node this connection leads from.
   m_connectFromNodeIndex = dataBuffer->readNextLong();
   DEBUG_DB(mc2dbg << "      m_connectFromNodeIndex="
            << m_connectFromNodeIndex<< endl);

   // Read vehicles that are allowed traversing this connection
   m_vehicleRestriction = dataBuffer->readNextLong();
   DEBUG_DB(mc2dbg << "      m_vehicleRestriction=" << m_vehicleRestriction 
                 << endl);

   // Read turn-direction, crossing kind, exit count and nbr signposts.
   m_turnDirection = ItemTypes::turndirection_t( dataBuffer->readNextByte() );
   m_crossingKind = ItemTypes::crossingkind_t( dataBuffer->readNextByte() );
   m_exitCount =  dataBuffer->readNextByte();


   // Read all sign posts, not used anymore.
   // This makes it possible to still load mcm version 7 and previous.
   byte nbrSignPosts = dataBuffer->readNextByte(); // Not used anymore
   for (byte j=0; j<nbrSignPosts; j++) {
      dataBuffer->readNextLong();// Read but not store sign post string code
   }

   m_restrictionsExplicitlySet = false;


   DEBUG_DB(mc2dbg << "      m_turndirection=" << (int)m_turnDirection
            << endl);
   DEBUG_DB(mc2dbg << "      m_crossingkind=" << (int)m_crossingKind << endl;);
   DEBUG_DB(mc2dbg << "      m_exitCount=" << (int)m_exitCount << endl;);
   DEBUG_DB(mc2dbg << "      nbrSignPosts=" << (uint32) nbrSignPosts;);
   DEBUG_DB(mc2dbg << endl);
}

bool
OldConnection::save(DataBuffer *dataBuffer)
{
   DEBUG_DB(mc2dbg << "      OldConnection::save()" << endl;)

   dataBuffer->writeNextLong(m_connectFromNodeIndex);
   DEBUG_DB(mc2dbg << "      m_connectFromNodeIndex="
            << m_connectFromNodeIndex << endl;)

   dataBuffer->writeNextLong(m_vehicleRestriction);
   DEBUG_DB(mc2dbg << "      m_vehicleRestriction=" << m_vehicleRestriction 
                 << endl;)

   dataBuffer->writeNextByte(m_turnDirection);
   dataBuffer->writeNextByte(m_crossingKind);
   dataBuffer->writeNextByte(m_exitCount);
   DEBUG_DB(mc2dbg << "      m_turnDirection=" << uint32(m_turnDirection) 
                 << endl;);
   DEBUG_DB(mc2dbg << "      m_crossingKind=" << uint32(m_crossingKind)
            << endl;);
   DEBUG_DB(mc2dbg << "      m_exitCount=" << uint32(m_exitCount) << endl;);

   // Not used anymore. Has been used for sing posts.
   dataBuffer->writeNextByte(0); 

   return (true);
}

OldConnection* 
OldConnection::createNewConnection(DataBuffer* dataBuffer, 
                                   OldGenericMap* theMap) 
{
   if (theMap != NULL) {
      OldConnection* con = static_cast<MC2Allocator<OldConnection> *>
         (theMap->m_connectionAllocator)->getNextObject();
      con->createFromDataBuffer(dataBuffer);
      return con;
   } else {
      return new OldConnection(dataBuffer);
   }
}


float64
OldConnection::getPenaltyFactor(byte roadClass, 
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
OldConnection::printTurnDirection()
{
   switch (m_turnDirection) {
      case (ItemTypes::UNDEFINED) :
         cout << "undefined" ;
         break;
      case (ItemTypes::LEFT) :
         cout << "left";
         break;
      case (ItemTypes::RIGHT) :
         cout << "right";
         break;
      case (ItemTypes::AHEAD) :
         cout << "ahead";
         break;
      case (ItemTypes::UTURN) :
         cout << "U-turn";
         break;
      case (ItemTypes::FOLLOWROAD) :
         cout << "follow road";
         break;
      default :
         cout << "???";
   }
}

void
OldConnection::printMidFile(ofstream& midFile)
{
   midFile << getConnectFromNode() << ":";
   
   //Check turn directions
   switch(getTurnDirection()){
      case ItemTypes::UNDEFINED : {
         midFile << "UND";
      } break;
      case ItemTypes::LEFT : {
         midFile << "LE";
      } break;
      case ItemTypes::AHEAD : {
         midFile << "AH";
      } break;
      case ItemTypes::RIGHT : {
         midFile << "RI";
      } break;
      case ItemTypes::UTURN : {
         midFile << "UT";
      } break;
      case ItemTypes::FOLLOWROAD : {
         midFile << "FO";
      } break;
      case ItemTypes::ENTER_ROUNDABOUT : {
         midFile << "NR";
      } break;
      case ItemTypes::EXIT_ROUNDABOUT : {
         midFile << "XR";
      } break;
      case ItemTypes::RIGHT_ROUNDABOUT : {
         midFile << "RR";
      } break;
      case ItemTypes::LEFT_ROUNDABOUT : {
         midFile << "LR";
      } break;
      case ItemTypes::AHEAD_ROUNDABOUT : {
         midFile << "AR";
      } break;
      case ItemTypes::ON_RAMP : {
         midFile << "OR";
      } break;
      case ItemTypes::OFF_RAMP : {
         midFile << "FR";
      } break;
      case ItemTypes::OFF_RAMP_LEFT : {
         midFile << "FRL";
      } break;
      case ItemTypes::OFF_RAMP_RIGHT : {
         midFile << "FRR";
      } break;
      case ItemTypes::ENTER_BUS : {
         midFile << "NB";
      } break;
      case ItemTypes::EXIT_BUS : {
         midFile << "XB";
      } break;
      case ItemTypes::CHANGE_BUS : {
         midFile << "CB";
      } break;
      case ItemTypes::KEEP_RIGHT : {
         midFile << "KR";
      } break;
      case ItemTypes::KEEP_LEFT : {
         midFile << "KL";
      } break;
      case ItemTypes::ENTER_FERRY : {
         midFile << "NF";
      } break;
      case ItemTypes::EXIT_FERRY : {
         midFile << "XF";
      } break;
      case ItemTypes::CHANGE_FERRY : {
         midFile << "CF";
      } break;
      case ItemTypes::MULTI_CONNECTION_TURN : {
         midFile << "MCT";
      } break;
      
   }

   midFile << ":";
   
   //Check crossing kinds for current connection
   switch(getCrossingKind()){
      case ItemTypes::UNDEFINED_CROSSING : {
         midFile << "UNC";
      } break;
      case ItemTypes::NO_CROSSING : {
         midFile << "NCR";
      } break;
      case ItemTypes::CROSSING_3WAYS_T : {
         midFile << "C3WT";
      } break;
      case ItemTypes::CROSSING_3WAYS_Y : {
         midFile << "C3WY";
      } break;
      case ItemTypes::CROSSING_4WAYS : {
         midFile << "C4W";
      } break;
      case ItemTypes::CROSSING_5WAYS : {
         midFile << "C5W";
      } break;
      case ItemTypes::CROSSING_6WAYS : {
         midFile << "C6W";
      } break;
      case ItemTypes::CROSSING_7WAYS : {
         midFile << "C7W";
      } break;
      case ItemTypes::CROSSING_8WAYS : {
         midFile << "C8W";
      } break;
      case ItemTypes::CROSSING_2ROUNDABOUT : {
         midFile << "C2R";
      } break;
      case ItemTypes::CROSSING_3ROUNDABOUT : {
         midFile << "C3R";
      } break;
      case ItemTypes::CROSSING_4ROUNDABOUT : {
         midFile << "C4R";
      } break;
      case ItemTypes::CROSSING_4ROUNDABOUT_ASYMMETRIC : {
         midFile << "C4RA";
      } break;
      case ItemTypes::CROSSING_5ROUNDABOUT : {
         midFile << "C5R";
      } break;
      case ItemTypes::CROSSING_6ROUNDABOUT : {
         midFile << "C6R";
      } break;
      case ItemTypes::CROSSING_7ROUNDABOUT : {
         midFile << "C7R";
      } break;
      default : {
         midFile << "No such crossing";
      }
   }
   
   for(uint32 type = 0; type < 21; type++){
      if (!isVehicleAllowed(ItemTypes::vehicle_t(type))){     
         midFile << ":" << type;
      }     
   }
}

void 
OldConnection::setExitCount( byte exitCount )
{
   m_exitCount = exitCount;
}

void 
OldConnection::setFromNode( uint32 fromNodeID )
{
   m_connectFromNodeIndex = fromNodeID;
}


bool OldConnection::getRestrictionExplicitlySet() const{
   return m_restrictionsExplicitlySet;
}

void OldConnection::setRestrictinoExplicitlySet(bool value){
   m_restrictionsExplicitlySet = value;
}


uint32 
OldConnection::getMemoryUsage(void) const 
{
   uint32 sizeOfSignPosts = 0;
   return (sizeof(OldConnection) + sizeOfSignPosts);
}

void 
OldConnection::setCrossingKind(ItemTypes::crossingkind_t x) 
{
   m_crossingKind = x;
}

void 
OldConnection::setVehicleRestrictions( uint32 vehicleRestriction ) 
{
   m_vehicleRestriction = vehicleRestriction;
}

void 
OldConnection::resetVehicleRestrictions() 
{
   m_vehicleRestriction = MAX_UINT32;
}

void 
OldConnection::addVehicleRestrictionsAll() 
{
   // Should be done with masking as below, but for now just
   // deny all vehicles (including pedestrians and bicycles)
   m_vehicleRestriction = 0;
   
   /*
   uint32 mask = 0;
   mask = VEHICLEREST_MASK_CAR | 
          VEHICLEREST_MASK_TRUCK | 
          VEHICLEREST_MASK_BUS;
                     
   m_vehicleRestriction &= (~mask);
   */
}

void 
OldConnection::addVehicleRestrictions(uint32 t) 
{
   m_vehicleRestriction &= (~t);
}

bool
OldConnection::updateConnAttributesFromConn(
            OldConnection* otherConn )
{
   bool retVal = false;

   if (otherConn == NULL)
      return retVal;

   // fromNode is not changed
   // vehicleRestrictions
   // turnDirection
   // crossingKind
   // exitCount
   // signPosts if same map
   
   if (m_vehicleRestriction != otherConn->getVehicleRestrictions()) {
      m_vehicleRestriction = otherConn->getVehicleRestrictions();
      mc2dbg4 << "    changing vehicle restrictions to 0x" << hex
              << m_vehicleRestriction << dec << endl;
      retVal = true;
   }

   if (m_turnDirection != otherConn->getTurnDirection()) {
      m_turnDirection = otherConn->getTurnDirection();
      mc2dbg4 << "    changing turn directions to " 
         << StringTable::getString(getTurnStringCode(), StringTable::ENGLISH)
         << endl;
      retVal = true;
   }

   if (m_crossingKind != otherConn->getCrossingKind()) {
      m_crossingKind = otherConn->getCrossingKind();
      mc2dbg4 << "    changing crossing kind to " << m_crossingKind << endl;
      retVal = true;
   }

   if (m_exitCount != otherConn->getExitCount()) {
      m_exitCount = otherConn->getExitCount();
      mc2dbg4 << "    changing exit count to " << int(m_exitCount) << endl;
      retVal = true;
   }

   return retVal;
}

byte 
OldConnection::getNbrSignPost(const OldGenericMap& theMap, 
                              uint32 toNode) const
{
   const SignPostTable& spTable = theMap.getSignPostTable();
   return spTable.getNbrSignPosts(this->getFromNode(), toNode);
} // getNbrSignPost

const vector<GMSSignPost*>&
OldConnection::getSignPosts( const OldGenericMap& theMap, 
                             uint32 toNodeID ) const {
   return theMap.getSignPostTable().getSignPosts( 
      this->getFromNode(), toNodeID );
}

