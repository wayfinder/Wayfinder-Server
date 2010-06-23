/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DISTURBANCEPACKET_H
#define DISTURBANCEPACKET_H

#define DISTURBANCE_REQUEST_PRIO DEFAULT_PACKET_PRIO

#include "Packet.h"
#include "DisturbanceElement.h"
#include <map>
#include "DisturbancePacketUtility.h"


/**
 *   Creates a DisturbanceRequestPacket.
 *
 */
class DisturbanceRequestPacket : public RequestPacket
{
  static const int START_TIME_POS = REQUEST_HEADER_SIZE;
  static const int END_TIME_POS = REQUEST_HEADER_SIZE + 4;
  static const int MAP_ID_POS = REQUEST_HEADER_SIZE + 8;
  
  public:
   
   /**
    *  Returns all the disturbances on a map which are active within
    *  a time interval.
    *  @param packetID The packetID.
    *  @param requestID The ID of the request.
    *  @param startTime The start time of the time interval.
    *  @param endTime The end time of the time interval.
    *  @param mapID The mapID 
    */
   DisturbanceRequestPacket(uint32 packetID,
                            uint32 requestID,
                            uint32 startTime,
                            uint32 endTime,
                            uint32 mapID,
                            uint32 originIP = 0,
                            uint16 originPort = 0);

   /**
    *  Destructor
    *
    */
   virtual ~DisturbanceRequestPacket();

   /**
    *  Returns the startTime of the interval
    *  @return The startTime of the interval.
    */
   uint32 getStartTime() const;

   /**
    *  Returns the endTime of the interval
    *  @return The endTime of the interval.
    */
   uint32 getEndTime() const;

   /**
    *  Returns the requested mapID.
    *  @return The requested mapID.
    */
   uint32 getRequestedMapID() const;

  private:
};



/// ReplyPacket

/**
 *   Creates a DisturbanceReplyPacket
 *
 *
 */
class DisturbanceReplyPacket : public ReplyPacket
{
  public:

   /**
    *   Returns a a map with the disturbances on a map.
    *   @param p The DisturbanceRequestPacket.
    *   @param distMap The map with the disturbances.
    *   @param removeDisturbances True if the disturbances are going to
    *   be removed.
    */
   DisturbanceReplyPacket(const DisturbanceRequestPacket* p,
                          const IDToDisturbanceMap& distMap,
                          bool removeDisturbances,
                          bool removeAll = false);
   
   /**
    *  Destructor
    *
    */
   virtual ~DisturbanceReplyPacket();

   /**
    *  Returns the disturbances.
    *  @param distMap  The map with the disturbances.
    *  @param removedDisturbance True if the disturbances are going to
    *  be removed.
    */ 
   void getDisturbances(map<uint32,DisturbanceElement*> &distMap,
                        bool &removeDisturbances,
                        bool &removeAll) const;

};


#endif















