/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTEREADER_H
#define ROUTEREADER_H

#include "config.h"
#include "StandardReader.h"
#include <vector>

class Head;
class PushService;

/**
 * Class that handles some packets that are sent to the 
 * RouteModule-leader in a special way.
 *
  */
class RouteReader : public StandardReader {

   public :
      /**
       * Creates a RouteReader according to the specifications.
       *   
       * @param   q              The queue into which we insert
       *                         the request to be handled by
       *                         the processor.
       * @param   loadedMaps     Where to look for the currently
       *                         loaded maps.
       * @param   preferedPort   The port to use if possible
       * @param   definedRank    Value used when voting for leader.
       *                         High value indicated that the system 
       *                         administrator want this Module to be
       *                         leader.
       * @param logName          is the name of a logfile if it exists.
       * @param superLeader      <code>True</code> if the RouteModule
       *                         should act as the old kind of leader
       *                         and not send on
       *                         SubRouteRequests to the availables.
       *                         <code>False</code> for testing new 
       *                         leader-is-not-server functionality.
       * @param wantedServices   Subscription services wanted by the
       *                         module.
       * @param waitForPush      <code>True</code> if the RouteModule should
       *                         save the packets for a map until it has
       *                         received a push packets for the map.
       */
      RouteReader(Queue* q,
                  NetPacketQueue& sendQueue,
                  MapSafeVector* loadedMaps,
                  PacketReaderThread* packetReader,
                  uint16 port,
                  uint32 definedRank,
                  char* logName,
                  bool superLeader,
                  vector<PushService*>* wantedServices,
                  bool waitForPush);

      /**
       * Deletes the RouteReader and releases the allocated memory.
       */
      virtual ~RouteReader();
      
  protected:
   /**
    * This module doesn't use country maps.
    */
   virtual bool usesCountryMaps() const { return false; }
   
  private:
      /**
       * Take care of some of the special control packets that 
       * might be received by the RouteModule-leader. Calls the 
       * leaderProcessCtrlPacket in the super class to take care of 
       * the ``ordinary'' control packet.
       *
       * @param   p is the incoming control packet.
       * @return  true if the packet was handled here or by the
       *          method in the superclass, false otherwise.
       */
      bool leaderProcessCtrlPacket( Packet* p );
      
      /**
       * Takes care of the non control packets sent to the route leader.
       * Handles the RouteRequestPacket.
       * NB! Deletes the packet or puts it in local queue for processing.
       *
       * @param  packet is the incoming packet to process.
       * @return true if the packet was handled here, false if not.
       */
      bool leaderProcessNonCtrlPacket( Packet* packet );

      /**
       *   Prints some information about the origin and destinations
       *   supplied.
       *   @param originList The list of origins.
       *   @param destList   The list of destinations.
       */
      void printOrigDests(Head* originList,
                          Head* destList);


      /** 
       * The name of the logfile.
       */
      char* m_logName;

      /**
       *   True if the leader should act as a server and refuse to
       *   send incoming SubRoutePackets to the availables.
       */
      bool m_superLeader;
}; 

#endif


