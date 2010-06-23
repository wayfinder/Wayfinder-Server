/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODULE_PACKET_H
#define MODULE_PACKET_H

#include "config.h"
#include "Packet.h"

class IPnPort;
class Balancer;

#define MAX_VOTE_LENGTH      MAX_PACKET_SIZE
#define VOTE_PRIO             0

#define MAX_HEARTBEAT_LENGTH MAX_PACKET_SIZE
#define HEARTBEAT_PRIO        0


 /** 
   *  Class for handling the voting packets. 
   * 
   *  In addition to the normal header this packet contains
   *  \begin{tabular}{lll}
   *     Pos & Size    & \\ \hline
   *     HEADER_SIZE   & 4 bytes & Rank
   *  \end{tabular}
   */
class VotePacket : public Packet {
public:

   /**
    *   Creates a new VotePacket with the supplied <code>ip</code>, <code>
    *   port</code> and <code>rank</code>.
    *   @param ip     The ip of the sender.
    *   @param port   The ownport of the sender.
    *   @param rank   The rank of the sender.
    *   @param direct True if the packet is sent directly to the receiver.
    */
   VotePacket(uint32 ip, uint16 port, int32 rank, bool direct );

   /**
    *   Returns the rank of the packet.
    *   @return The rank of the sender of the packet.
    */
   int32 getRank() const;

   /**
    *   Returns "direct" or "multicast" depending on the value in the
    *   packet.
    */
   const char* directOrMultiDbg() const;
   
   /**
    *   Use this method to find out if this module is better
    *   than the sender of this packet. It compares rank, ip 
    *   and finally portnumber. 
    *   @return 1 if this module is better leader. \\
    *          -1 if the sender of the packet is better leader \\
    *           0 if they are equal. This should never happen.
    */
   int betterThanThis(int32 myRank, uint16 myPort) const;
};
 
/**
  *   Contains a list of the modules known by the leader
  * 
  *   In addition to the normal header this packet contains
  *   \begin{tabular}{lll}
  *      Pos         & Size    & \\ \hline
  *      HEADER_SIZE & 4 bytes & Size of list \\
  *      for each    & 4 bytes & IP \\
  *                  & 2 bytes & Port
  *   \end{tabular}
  */
class HeartBeatPacket : public Packet {
public:

   /**
    *    @param ip         The IP which the answer should be sent to.
    *    @param port       The portnumber.
    *    @param rank       The rank of the module.
    *    @param balancer   The load balancer of the module.
    */
   HeartBeatPacket(const IPnPort& sender,
                   int32 rank,
                   const Balancer* balancer);
      
      /**
       *   @return The rank of the sending leader
       */
      int32 getRank() const;

      /**
       *   Get the number of availables that the leader knows
       *   about.
       *   @return The number of modules in the HeartBeatPacket.
       */
      int getNbrModules() const;
};

/**
  *   Packet used when the leader wants a Module to change the map.
  */
class ChangeMapPacket : public Packet {
   public:

      /**
        *   @param   ip The IP which the answer should be sent to.
        *   @param   port The portnumber for the answer.
        *   @param   mapID The new mapID to load.
        *   @param   removeMapID {\bf If} it's nessecery to remove
        *            any map then the map with this id should be removed.
        */
      ChangeMapPacket(  uint32 ip, 
                        uint16 port, 
                        uint32 newMapID, 
                        uint32 removeMapID);

      /**
        *   @return  The ID of the map to be loaded.
        */
      uint32 getNewMapID();

      /**
        *   @return  The ID of the map that could be removed.
        */
      uint32 getRemoveMapID();
};

/*
class MapChangedPacket : public Packet {
   public:

      MapChangedPacket();

      

};
*/

/**
  *   "Flag" packet to tell the threads in the module to shut
  *   down cleanly.
  * 
  *   This packets has no additional content, only a header.
  */
class ShutdownPacket : public Packet {
   public:

     /**
      *    Create a ShutdownPacket
      */
      ShutdownPacket();
};

#endif // MODULE_PACKET_H

