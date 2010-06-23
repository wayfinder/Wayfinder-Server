/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//#include "Connection.h"

#ifndef STREETSEGMENTITEMPACKET_H
#define STREETSEGMENTITEMPACKET_H

#include "config.h"
#include "Packet.h"
#include "StringUtility.h"
#include "TrafficDataTypes.h"

#define STREETSEGMENTITEM_REPLY_MAX_LENGTH    2000

/** 
 */
class StreetSegmentItemRequestPacket : public RequestPacket {
  public:
   /**
    * Creates a StreetSegmentItemRequestPacket.
    * @param packetID The packetID.
    * @param requestID The ID of the request that created the packet.
    * @param index The index of the packet, if it's the first or second..
    * @param lat The latitude of the TMC-point.
    * @param lon The latitude of the TMC-point.
    * @param originIP The IP number of the sender.
    * @param originPort The port number of the sender.
    */
   StreetSegmentItemRequestPacket(uint16 packetID,
                                  uint16 requestID,
                                  uint32 index,
                                  int32 lat,
                                  int32 lon,
                                  TrafficDataTypes::direction dir,
                                  uint32 originIP = 0,
                                  uint16 originPort = 0);
   
   /**
    *  Returns the index of the packet.
    *  @return The index.
    */
   uint32 getIndex() const;
   
   /**
    *  Returns the latitude of the TMC-point.
    *  @return  The latitude of the TMC-point.
    */
   int32 getLatitude() const;
  
   /**
    *  Returns the longitude of the TMC-point.
    *  @return  The longitude of the TMC-point.
    */
   int32 getLongitude() const;

   TrafficDataTypes::direction getDirection() const;
};


class StreetSegmentItemReplyPacket : public ReplyPacket {
  public:
   
   /**
    * Creates a StreetSegmentItemReplyPacket.
    * @param p The StreetSegmentItemRequestPacket.
    * @param status The status of the packet.
    * @param lat1 The latitude of the point in the positive direction.
    * @param lon1 The longitude of the point in the positive direction.
    * @param angle1 The angle for the point in the positive direction.
    * @param lat2 The latitude of the point in the negative direction.
    * @param lon2 The longitude of the point in the negative direction.
    * @param angle2 The angle for the point in the negative direction.
    * @param mapID The mapID where the disturbance is located, if one point.
    * @param nodeID The nodeID where the disturbance starts, if one point.
    * @param costC The extra cost for the route module.    
    */
   StreetSegmentItemReplyPacket(const StreetSegmentItemRequestPacket* p,
                                uint32 status,
                                uint32 index,
                                uint32 distance,
                                uint32 mapID,
                                int32 lat,
                                int32 lon,
                                uint32 firstNodeID,
                                uint32 secondNodeID,
                                uint32 firstAngle,
                                uint32 secondAngle);

   // Returns the index
   uint32 getIndex() const;
   
   // Returns the distance
   uint32 getDistance() const;

   // Returns the mapID
   uint32 getMapID() const;

   int32 getLatitude() const;

   int32 getLongitude() const;
   
   // Returns the nodeID
   uint32 getFirstNodeID() const;

   uint32 getSecondNodeID() const;

   // Returns the angle
   uint32 getFirstAngle() const;

   uint32 getSecondAngle() const;
   
   inline void addMapID(uint32 mapID);
   inline uint32 getNbrMapIDs() const;
   inline uint32 getMapID(uint32 i) const;
};

inline void
StreetSegmentItemReplyPacket::addMapID(uint32 mapID)
{
   uint32 nbrMaps = getNbrMapIDs();
   int position = REPLY_HEADER_SIZE + 40 + 4 * nbrMaps;
   incWriteLong(position, mapID);
   setLength(position);
   writeLong(REPLY_HEADER_SIZE + 36, getNbrMapIDs()+1);
}

inline uint32 
StreetSegmentItemReplyPacket::getNbrMapIDs() const
{
   return readLong(REPLY_HEADER_SIZE + 36);   
}

inline uint32 
StreetSegmentItemReplyPacket::getMapID(uint32 i) const
{
   if( (i+1) > getNbrMapIDs() ) {
      return MAX_UINT32;
   } else {
      int position = REPLY_HEADER_SIZE + 40 + 4 * i;
      return readLong(position);
   }
}
#endif

