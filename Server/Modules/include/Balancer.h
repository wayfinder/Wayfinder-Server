/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BALANCER_H
#define BALANCER_H

#include "config.h"

#include "PacketSendList.h"
#include "MapBits.h"
#include <set>

class StatisticsPacket;
class Packet;
class RequestPacket;

/** 
 *   Basic class for load balancing among modules.
 *   Can be used directly for testing, but should not be used
 *   for any other purposes. Use only derived classes,
 *   eg LRUBalancer.or xxx instead.
 */
class Balancer {
public:

   /**
    *   Destructoir.
    */
   virtual ~Balancer() {}
   
   /**
    *   Updates the statistics. Currently also deletes the packet.
    *   @param packet A pointer to a StatisticsPacket.
    *   @param packetList Can be filled with packets to send in response
    *                     to the statistics packet, such as deleting maps
    *                     if duplicates are noticed.
    *   @return true if the statistics were used, false if ignored
    */
   virtual bool updateStats(StatisticsPacket* packet,
                            PacketSendList& packetList) = 0;
   
   /**
    *   Returns the number of known modules. Used in
    *   HeartBeatPackets.
    */
   virtual int getNbrModules() const = 0;

   /**
    *   Tells the Balancer that the module has become leader.
    */
   virtual void becomeLeader() = 0;
   
   /**
    *   Tells the Balancer that the module has become available.
    */
   virtual void becomeAvailable() = 0;
  
   /**
    *   Remove any modules that we haven't heard from for a while
    *   @param ownStatistics Statistics packet for this module,
    *                        shall be deleted by this function.
    *   @param packets Will be filled with packets to send.
    *   @return Milliseconds until next wanted timeout?
    */
   virtual int timeout( StatisticsPacket* ownStatistics, 
                        PacketSendList& packets ) = 0;
   
   /**
    *  Get packets with destination module to send for a certain
    *  RequestPacket.
    *  @param packetList The list of packets. On return it contains
    *                    the packets to send together with their
    *                    destinations.
    *  @param req        The request packet. Do not use it after
    *                    calling the function.
    */
   virtual bool getModulePackets(PacketSendList& packets,
                                 RequestPacket* req) = 0;

   /**
    *  Does the module use maps?
    *  @return Whether the module uses maps.
    */
   virtual bool usesMaps() const = 0;

   /**
    *  For modules using maps this function is used to let the
    *  balancer know which maps are available.
    *  @param allMaps A set of all maps.
    */
   virtual void setAllMaps( const set<MapID>& allMaps ) = 0;
};

#endif // BALANCER_H
