/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPSUBSCRIPTIONPACKET_H
#define MAPSUBSCRIPTIONPACKET_H

#define MAPSUBSCRIPTION_REQUEST_PRIO          (DEFAULT_PACKET_PRIO)
#define MAPSUBSCRIPTIONHEARTBEAT_REQUEST_PRIO (DEFAULT_PACKET_PRIO)

#include "config.h"
#include<map>
#include<vector>
#include<set>

#include "PushPacket.h"


/**
 *   Packet that informs a module that a subscription is wanted.
 *   Currently it is only possible to subscribe to all information
 *   from the type of module that this packet is sent to (for the
 *   specified maps).
 *
 */
class MapSubscriptionRequestPacket : public PushPacket {

public:

   /**
    *   Create a new MapSubscriptionRequestPacket
    *   @param mapID        One of the mapID:s in <code>wantedMaps</code>.
    *   @param serviceID    The service id.
    *   @param wantedMaps   A map with the id of the wanted maps
    *                       in first and last update time in second.
    *   @param unwantedMaps Vector of maps for which the subscription
    *                       should stop.
    */
   MapSubscriptionRequestPacket(uint32 mapID,
                                uint32 serviceID,
                                const map<uint32,uint32>& wantedMaps,
                                const vector<uint32>& unwantedMaps);
      

   /**
    *   @param wantedMaps   A map with the id of the wanted maps
    *                       in first and last update time in second.
    *   @param unwantedMaps Vector of maps for which the subscription
    *                       should stop.
    */
   void getData(map<uint32, uint32>& wantedMaps,
                vector<uint32>& unwantedMaps) const;
   
private:
   
   /**
    *   Calculates the size of the packet.
    */
   static int calcPacketSize(const map<uint32,uint32>& wantedMaps,
                             const vector<uint32>& unwantedMaps);
   
};

/**
 *   Packet to be sent from a module providing subscription content
 *   to a module that is subscribing to the content.
 */
class MapSubscriptionHeartbeatRequestPacket : public PushPacket {

public:

   /**
    *    Constructor. No request ID and packetID:s are used, since
    *    this kind of packets should not be included in requests.
    *    @param subscribed   Set of successfully subscribed maps.
    *    @param unsubscribed Set of recently removed maps (so if we
    *                        know that a map has been removed, we
    *                        do not have to wait for the next one.
    */
   MapSubscriptionHeartbeatRequestPacket(uint32 serviceID,
                                         const set<uint32>& subscribed,
                                         const set<uint32>& unsubscribed);

   /**
    *    Puts the 
    *
    */
   void getData(set<uint32>& subscribed,
                set<uint32>& unsubscribed) const;

   /**
    *    The time between the heartbeats for this kind of packets.
    */
   static const int HEARTBEAT_TIME_MS = 15000;
   
private:

   /**
    *   Calculates the packet size from the supplied input.
    *   @see MapSubscriptionHeartbeatRequestPacket.
    */
   static int calcPacketSize(const set<uint32>& subscribed,
                             const set<uint32>& unsubscribed);
   
};

#endif
