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
#include "TrafficDataTypes.h"
#include "MC2String.h"

class AddDisturbanceRequestPacket;
class Packet;
/**
 * Data class for AddDisturbanceRequestPacket.
 */
class AddDisturbanceRequestData
{
public:
   typedef uint8 ExtentType;
   static const ExtentType MAX_EXTENT;
   /** Default constructor. */
   AddDisturbanceRequestData() :
      m_disturbanceID(MAX_UINT32),
      m_type(TrafficDataTypes::NoType), 
      m_phrase(TrafficDataTypes::NoPhrase),        
      m_eventCode(0),                       
      m_startTime(0),                       
      m_endTime(0),                         
      m_creationTime(0),                    
      m_severity(TrafficDataTypes::NoSeverity),    
      m_direction(TrafficDataTypes::NoDirection),  
      m_extent(0),                           
      m_costFactor(0),                      
      m_queueLength(0)
   {}

   /**
    *  Creates a new AddDisturbanceRequestData object that can be used to
    *  construct new AddDisturbanceRequestPackets.
    *  @param packetID     The packetID
    *  @param requestID    The requestID
    *  @param size         The size of the data
    *  @param type         The disturbance type
    *  @param phrase       The phrase
    *  @param eventCode
    *  @param startTime    The start time
    *  @param endTime      The end time
    *  @param creationTime
    *  @param severity     The severity
    *  @param direction    The direction
    *  @param extent       The extent.
    *  @param costFactor
    *  @param queueLength
    */
   AddDisturbanceRequestData( TrafficDataTypes::disturbanceType type,
                              TrafficDataTypes::phrase phrase,
                              uint32 eventCode,
                              uint32 startTime,
                              uint32 endTime,
                              uint32 creationTime,
                              TrafficDataTypes::severity severity,
                              TrafficDataTypes::direction direction,
                              ExtentType extent,
                              uint32 costFactor,
                              uint32 queueLength ) :
      m_disturbanceID(MAX_UINT32),
      m_type(type), 
      m_phrase(phrase),        
      m_eventCode(eventCode),                       
      m_startTime(startTime),                       
      m_endTime(endTime),                         
      m_creationTime(creationTime),                    
      m_severity(severity),    
      m_direction(direction),  
      m_extent(extent),                           
      m_costFactor(costFactor),                      
      m_queueLength(queueLength)
   {}

   void addStrings(const MC2String& firstLocation,
                   const MC2String& secondLocation,
                   const MC2String& situationReference,
                   const MC2String& text)
   {
      m_firstLocation      = firstLocation;
      m_secondLocation     = secondLocation;
      m_situationReference = situationReference;
      m_text               = text;
   }

   void addDisturbance(uint32 nodeID,     
                       int32  lat,
                       int32  lon,
                       uint32 angle,
                       uint32 routeIndex)
   {
      m_nodes.push_back(DisturbanceNode(nodeID, lat, lon, angle, routeIndex));
   }

   void addDisturbance(uint32 nodeID,     
                       pair<int32,int32> latlon,
                       uint32 angle,
                       uint32 routeIndex)
   {
      addDisturbance( nodeID, latlon.first, latlon.second, angle, routeIndex );
   }


   void setDisturbanceID(uint32 disturbanceID)
   {
      m_disturbanceID = disturbanceID;
   }

private:
   int getSizeInPacket() const;
public:

   /**
    *   Saves the parameters into the packet.
    */
   int save(Packet* packet, int& pos) const;

   /**
    *   Loads the parameters from the packet.
    */
   int load(const Packet* packet, int& pos);

private:
   friend class AddDisturbanceRequestPacket;

   uint32                            m_disturbanceID;
   TrafficDataTypes::disturbanceType m_type;
   TrafficDataTypes::phrase          m_phrase;
   uint32                            m_eventCode;
   uint32                            m_startTime;
   uint32                            m_endTime;
   uint32                            m_creationTime;
   TrafficDataTypes::severity        m_severity;
   TrafficDataTypes::direction       m_direction;
   ExtentType                        m_extent;
   uint32                            m_costFactor;
   uint32                            m_queueLength;

   MC2String m_firstLocation;
   MC2String m_secondLocation;
   MC2String m_situationReference;
   MC2String m_text;
   
   struct DisturbanceNode {
      DisturbanceNode( uint32 nodeID,
                       int32  lat,
                       int32  lon,
                       uint32 angle,
                       uint32 routeIndex ) : 
         nodeID(nodeID),
         lat(lat),
         lon(lon),
         angle(angle),
         routeIndex(routeIndex)
      {}

      uint32 nodeID;
      int32  lat;
      int32  lon;
      uint32 angle;
      uint32 routeIndex;
   };
   vector<DisturbanceNode> m_nodes;

};

