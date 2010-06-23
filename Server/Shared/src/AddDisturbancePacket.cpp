/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AddDisturbancePacket.h"
#include "AddDisturbanceData.h"
#include "DisturbanceElement.h"

// -----------------------   Request   -------------------------
#define ADD_DISTURBANCE_REQUEST_PRIO  1

const AddDisturbanceRequestPacket::ExtentType 
AddDisturbanceRequestPacket::MAX_EXTENT = MAX_UINT8;

AddDisturbanceRequestPacket::
AddDisturbanceRequestPacket(uint16 packetID,
                            uint32 requestID,
                            uint32 size,
                            TrafficDataTypes::disturbanceType type,
                            TrafficDataTypes::phrase phrase,
                            uint32 eventCode,
                            uint32 startTime,
                            uint32 endTime,
                            uint32 creationTime,
                            TrafficDataTypes::severity severity,
                            TrafficDataTypes::direction direction,
                            uint8 extent,
                            uint32 costFactor,
                            uint32 queueLength)
   
      : RequestPacket( REQUEST_HEADER_SIZE + size,
                       ADD_DISTURBANCE_REQUEST_PRIO,
                       PACKETTYPE_ADDDISTURBANCEREQUEST,
                       packetID,
                       requestID,
                       MAX_UINT32)
{
   init( AddDisturbanceRequestData( type, phrase, eventCode, 
                                    startTime, endTime, creationTime, 
                                    severity, direction, extent,
                                    costFactor, queueLength) );
}

AddDisturbanceRequestPacket::
AddDisturbanceRequestPacket( uint16 packetID,
                             uint32 requestID,
                             uint32 size,
                             const AddDisturbanceRequestData& params ) 
      : RequestPacket( REQUEST_HEADER_SIZE + size,
                       ADD_DISTURBANCE_REQUEST_PRIO,
                       PACKETTYPE_ADDDISTURBANCEREQUEST,
                       packetID,
                       requestID,
                       MAX_UINT32)
{
   init(params);
}

AddDisturbanceRequestPacket::
AddDisturbanceRequestPacket( uint16 packetID,
                             uint32 requestID,
                             const AddDisturbanceRequestData& params ) 
      : RequestPacket( REQUEST_HEADER_SIZE + params.getSizeInPacket(),
                       ADD_DISTURBANCE_REQUEST_PRIO,
                       PACKETTYPE_ADDDISTURBANCEREQUEST,
                       packetID,
                       requestID,
                       MAX_UINT32)
{
   int pos = REQUEST_HEADER_SIZE;
   params.save(this, pos);
}


void AddDisturbanceRequestPacket::init(const AddDisturbanceRequestData& params)
{
   int position = REQUEST_HEADER_SIZE;
   incWriteLong(position, 0);  // Nbr of disturbances
   incWriteLong(position, MAX_UINT32); // To be set later
   incWriteLong(position, params.m_type);
   incWriteLong(position, params.m_phrase);
   incWriteLong(position, params.m_eventCode);
   incWriteLong(position, params.m_startTime);
   incWriteLong(position, params.m_endTime);
   incWriteLong(position, params.m_creationTime);
   incWriteLong(position, params.m_severity);
   incWriteLong(position, params.m_direction);
   incWriteLong(position, params.m_extent);
   incWriteLong(position, params.m_costFactor);
   incWriteLong(position, params.m_queueLength);
   
   setLength(position);
}

void
AddDisturbanceRequestPacket::addDisturbance(uint32 nodeID,     
                                            int32  lat,
                                            int32  lon,
                                            uint32 angle,
                                            uint32 routeIndex)
{
   int position = REQUEST_HEADER_SIZE;
   uint32 nbrOfDisturbances = readLong(position);
   writeLong(position, nbrOfDisturbances + 1);
   
   position = getLength();
   incWriteLong(position, nodeID);
   incWriteLong(position, lat);
   incWriteLong(position, lon);
   incWriteLong(position, angle);
   incWriteLong(position, routeIndex);
   setLength(position);
}

void
AddDisturbanceRequestPacket::addStrings(const MC2String& firstLocation,
                                        const MC2String& secondLocation,
                                        const MC2String& situationReference,
                                        const MC2String& text)
{
   int position = getLength();
   incWriteString(position, firstLocation.c_str());
   incWriteString(position, secondLocation.c_str());
   incWriteString(position, situationReference.c_str());
   incWriteString(position, text.c_str());
   
   setLength(position);
}

DisturbanceElement*
AddDisturbanceRequestPacket::getDisturbance() const
{
   int position = REQUEST_HEADER_SIZE;
   uint32 nbrDisturbances = incReadLong(position);
   uint32 disturbanceID = incReadLong(position);
   TrafficDataTypes::disturbanceType type =
      TrafficDataTypes::disturbanceType(incReadLong(position));
   TrafficDataTypes::phrase phrase =
      TrafficDataTypes::phrase(incReadLong(position));
   uint32 eventCode = uint32(incReadLong(position));
   uint32 startTime = incReadLong(position);
   uint32 endTime = incReadLong(position);
   uint32 creationTime = incReadLong(position);
   TrafficDataTypes::severity severity =
      TrafficDataTypes::severity(incReadLong(position));
   TrafficDataTypes::direction direction =
      TrafficDataTypes::direction(incReadLong(position));
   uint32 extent = incReadLong(position);
   uint32 costFactor = incReadLong(position);
   uint32 queueLength = incReadLong(position);
   
   vector<uint32> nodeVector;
   vector<int32> latVector;
   vector<int32> lonVector;
   vector<uint32> angleVector;
   vector<uint32> indexVector;
   
   for(uint32 i = 0; i < nbrDisturbances; i++)
   {
      uint32 nodeID = incReadLong(position);
      nodeVector.push_back(nodeID);
      int32 lat = incReadLong(position);
      latVector.push_back(lat);
      int32 lon = incReadLong(position);
      lonVector.push_back(lon);
      uint32 angle = incReadLong(position);
      angleVector.push_back(angle);
      uint32 routeIndex = incReadLong(position);
      indexVector.push_back(routeIndex);
   }

   char* temp;
   MC2String situationReference, text;
   MC2String firstLocation, secondLocation;
   
   incReadString(position, temp);
   firstLocation = temp;
   incReadString(position, temp);
   secondLocation = temp;
   incReadString(position, temp);
   situationReference = temp;
   incReadString(position, temp);
   text = temp;
      
   DisturbanceElement* distElem = new DisturbanceElement(disturbanceID,
                                                         situationReference,
                                                         type,
                                                         phrase,
                                                         eventCode,
                                                         startTime,
                                                         endTime,
                                                         creationTime,
                                                         severity,
                                                         direction,
                                                         firstLocation,
                                                         secondLocation,
                                                         extent,
                                                         costFactor,
                                                         text,
                                                         queueLength);
   
   for(uint32 i = 0; i < nbrDisturbances; i++) {
      uint32 nodeID = nodeVector[i];
      int32 lat = latVector[i];
      int32 lon = lonVector[i];
      uint32 angle = angleVector[i];
      uint32 routeIndex = indexVector[i];
      distElem->addCoordinate(nodeID,
                              lat,
                              lon,
                              angle,
                              routeIndex);
   }
   distElem->setMapID(getMapID());
   
   return distElem;
}

void
AddDisturbanceRequestPacket::setDisturbanceID(uint32 disturbanceID)
{
   writeLong(REQUEST_HEADER_SIZE + 4, disturbanceID);
}


// -----------------------   Reply   -------------------------

AddDisturbanceReplyPacket::AddDisturbanceReplyPacket(const RequestPacket* p,
                                                     uint32 status,
                                                     uint32 disturbanceID)
      : ReplyPacket(REPLY_HEADER_SIZE + 4,
                    PACKETTYPE_ADDDISTURBANCEREPLY,
                    p,
                    status)
   
{
   int pos = REPLY_HEADER_SIZE;
   incWriteLong(pos, disturbanceID);
   setLength(pos);
}

uint32
AddDisturbanceReplyPacket::getDisturbanceID() const
{
   return uint32(readLong(REPLY_HEADER_SIZE));
}

AddDisturbanceReplyPacket::~AddDisturbanceReplyPacket()
{
   
}
