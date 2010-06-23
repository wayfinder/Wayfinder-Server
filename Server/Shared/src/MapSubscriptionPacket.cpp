/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapSubscriptionPacket.h"

//------------------------------------------------------------------
// MapSubscriptionRequestPacket
//------------------------------------------------------------------

int
MapSubscriptionRequestPacket::
calcPacketSize(const map<uint32,uint32>& wantedMaps,
               const vector<uint32>& unwantedMaps)
{
   uint32 size = PUSH_HEADER_SIZE;
   size += 4; // Number of wanted maps.
   size += wantedMaps.size() * 8;
   size += 4; // Number of not wanted maps.
   size += unwantedMaps.size() * 4;
   return size;
}

MapSubscriptionRequestPacket::
MapSubscriptionRequestPacket(uint32 mapID,
                             uint32 serviceID,
                             const map<uint32,uint32>& wantedMaps,
                             const vector<uint32>& unwantedMaps) 
      : PushPacket ( calcPacketSize(wantedMaps, unwantedMaps),
                     Packet::PACKETTYPE_MAPSUBSCRIPTIONREQUEST,
                     mapID,
                     serviceID,
                     MAX_UINT32) // Timestamp, not used here.
{
   setPriority(MAPSUBSCRIPTION_REQUEST_PRIO);
   int pos = PUSH_HEADER_SIZE;
   // Write size of wanted maps   
   incWriteLong(pos, wantedMaps.size());
   // Write wanted maps and update times
   
   for ( map<uint32,uint32>::const_iterator it = wantedMaps.begin();
         it != wantedMaps.end();
         ++it) {
      incWriteLong(pos, it->first);   // MapID
      incWriteLong(pos, it->second);  // Update time
   }
   // Write unwanted maps
   incWriteLong(pos, unwantedMaps.size());
   // Write unwanted maps
   for( vector<uint32>::const_iterator it = unwantedMaps.begin();
        it != unwantedMaps.end();
        ++it) {
      incWriteLong(pos, *it);
   }
   setLength(pos);
}

void
MapSubscriptionRequestPacket::getData(map<uint32, uint32>& wantedMaps,
                                      vector<uint32>& unwantedMaps) const
{
   int pos = PUSH_HEADER_SIZE;
   // Read the number of wanted maps
   int nbrWanted = incReadLong(pos);
   for (int i=0; i < nbrWanted; ++i ) {
      uint32 mapID = incReadLong(pos);
      uint32 updateTime = incReadLong(pos);
      wantedMaps.insert(pair<uint32,uint32>(mapID, updateTime));
   }
   // Read the number of unwanted maps
   int nbrNotWanted = incReadLong(pos);
   for ( int i=0; i < nbrNotWanted; ++i ) {
      unwantedMaps.push_back(incReadLong(pos));
   }
}

//------------------------------------------------------------------
// MapSubscriptionHeartbeatRequestPacket
//------------------------------------------------------------------

int
MapSubscriptionHeartbeatRequestPacket::
calcPacketSize(const set<uint32>& subscribed,
               const set<uint32>& unsubscribed)
{
   int size = PUSH_HEADER_SIZE;
   size += 4; // Nbr subscribed maps
   size += subscribed.size() * 4;
   size += 4; // Nbr unsubscribed maps
   size += unsubscribed.size() * 4;
   return size;
}

MapSubscriptionHeartbeatRequestPacket::
MapSubscriptionHeartbeatRequestPacket(uint32 serviceID,
                                      const set<uint32>& subscribed,
                                      const set<uint32>& unsubscribed)
      : PushPacket( calcPacketSize(subscribed, unsubscribed),
                    Packet::PACKETTYPE_MAPSUBSCRIPTIONHEARTBEATREQUEST,
                    MAX_UINT32, // MapID
                    serviceID, 
                    MAX_UINT32) // FIXME: timestamp, fix later
{
   int pos = PUSH_HEADER_SIZE;
   
   // Write the number of subscribed maps
   incWriteLong(pos, subscribed.size());
   for( set<uint32>::const_iterator it(subscribed.begin());
        it != subscribed.end();
        ++it) {
      incWriteLong(pos, *it);
   }
   
   // Write the number of unsubscribed maps
   incWriteLong(pos, unsubscribed.size());
   for( set<uint32>::const_iterator it(unsubscribed.begin());
        it != unsubscribed.end();
        ++it) {
      incWriteLong(pos, *it);
   }
      
   setLength(pos);
}


void
MapSubscriptionHeartbeatRequestPacket::getData(set<uint32>& subscribed,
                                               set<uint32>& unsubscribed) const
{
   int pos = PUSH_HEADER_SIZE;

   // Read the subscribed maps
   int nbrSubscribed = incReadLong(pos);
   for(int i=0; i < nbrSubscribed; ++i ) {
      subscribed.insert(incReadLong(pos));
   }

   // Read the unsubscribed maps
   int nbrUnsubscribed = incReadLong(pos);
   for ( int i=0; i < nbrUnsubscribed; ++i ) {
      unsubscribed.insert(incReadLong(pos));
   }
}
