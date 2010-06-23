/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTINGINFOPACKET_H
#define ROUTINGINFOPACKET_H

#define ROUTINGINFO_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define ROUTINGINFO_REPLY_PRIO   DEFAULT_PACKET_PRIO

#define ROUTINGINFO_REQUEST_MAX_LENGTH 65536
#define ROUTINGINFO_REPLY_MAX_LENGTH   65536

#include "config.h"
#include "Packet.h"

#include<map>


/// Map containing level in first and distance in meters in second.
class CriteriumMap : public map<int, uint32> {
};

/**
 *   The contents of the RoutingInfoPacket.
 *   Contains information that the RouteSender needs to
 *   be able to route.
 */
class RoutingInfo {
public:
   
   /**
    *   Creates a new RoutingInfo
    */
   RoutingInfo(const CriteriumMap& upCriteria,
               const CriteriumMap& downCriteria);

   /**
    *   Returns a reference to the up-crit.
    */
   const CriteriumMap& getUpCriteria() const;
      
   /**
    *   Returns a reference to the down-crit.
    */
   const CriteriumMap& getDownCriteria() const;

   /**
    *    Loads the RoutingInfo from the packet.
    *    @param p   Packet to load from.
    *    @param pos Position to start at. Will be updated.
    */
   bool load(const Packet* p, int& pos);
   
   /**
    *    Saves the RoutingInfo to the packet.
    *    @param p Packet to save the collection into.
    *    @param pos Position to start at. Will be updated.
    */
   bool save(Packet* p, int &pos) const;

   /**
    *    Returns the number of bytes that the namecollection
    *    will use in the packet.
    */
   int getSizeInPacket() const;
   
private:

   /**
    *    Empty constructor.
    */
   RoutingInfo() {};
   
   /**
    *    Saves a CriteriumMap into the packet.
    */
   static bool saveMap(Packet* p, int& pos, const CriteriumMap& theMap);

   /**
    *    Loads a criterium map from the packet.
    */
   static bool loadMap(const Packet* p, int& pos, CriteriumMap& theMap);
   
   /// Holds the up-criteria.
   CriteriumMap m_upCriteria;
   
   /// Holds the down-criteria.
   CriteriumMap m_downCriteria;

   /// To be able to use the private default constructor.
   friend class RoutingInfoReplyPacket;
};

class RoutingInfoRequestPacket : public RequestPacket {
public:

   /** Constructor. Sets packettype and so on */
   RoutingInfoRequestPacket();
};

class RoutingInfoReplyPacket : public ReplyPacket {
public:
   /**
    *   Creates a new RoutingInfoPacket containing the
    *   supplied info.
    *   @param req The request.
    *   @param info The contents of the packet.
    */
   RoutingInfoReplyPacket(const RoutingInfoRequestPacket* req,
                          const RoutingInfo& info);

   /**
    *   Returns the routing info of the packet.
    */
   RoutingInfo getRoutingInfo() const;
   
};

#endif
