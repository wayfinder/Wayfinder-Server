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

#include "IPnPort.h"

#include "NetUtility.h"

#include "ModulePacket.h"
#include "Balancer.h"




VotePacket::VotePacket( uint32 ip, uint16 port, int32 rank,
                        bool direct)
   : Packet(  MAX_VOTE_LENGTH,
              VOTE_PRIO,
              PACKETTYPE_VOTE,
              ip, port, 0, 0)
{   
   int packetPosition = HEADER_SIZE;
   incWriteLong( packetPosition, rank );
   incWriteLong( packetPosition, direct );
   // 5 seconds timeout.
   setTimeout( 5 );
   setLength(packetPosition);
}

int32 VotePacket::getRank() const
{
   return readLong( HEADER_SIZE );
}

const char*
VotePacket::directOrMultiDbg() const
{
   int pos = HEADER_SIZE;
   incReadLong( pos ); // Skip the rank
   bool direct = incReadLong( pos );
   return direct ? "Direct" : "Multicast";
}

int VotePacket::betterThanThis(int32 myRank, uint16 myPort) const
{
   uint32 localIP;
  
   mc2dbg8 << "[VotePacket] myRank: " << myRank << " packet rank: "
           << getRank() << endl;
   // First compare ranks 
   if(getRank() > myRank)
      return -1;
   if(getRank() < myRank)
      return 1;

   localIP = NetUtility::getLocalIP();
   if(getOriginIP() < localIP)
      return 1;
   if(getOriginIP() > localIP)
      return -1;

   // He's running on my computer. Compare ports
   if(getOriginPort() < myPort)
      return 1;
   if(getOriginPort() > myPort)
      return -1;

   // We should never be here!
   mc2log << error << "[VotePacket] BUG! betterThanThis(): "
          << "We have two modules with the same IP and port. Impossible!"
          << endl;

   return 0;

}

HeartBeatPacket::HeartBeatPacket( const IPnPort& sender,
                                  int32 rank,
                                  const Balancer* balancer)
      : Packet(  MAX_HEARTBEAT_LENGTH, HEARTBEAT_PRIO,
                 PACKETTYPE_HEARTBEAT, sender.getIP(), sender.getPort(), 0, 0)
{
   int packetPosition = HEADER_SIZE;
   incWriteLong( packetPosition, rank );      

   incWriteLong( packetPosition, balancer->getNbrModules() );
   
   setLength(packetPosition);
}

int32
HeartBeatPacket::getRank() const
{
   return readLong( HEADER_SIZE );
}

int
HeartBeatPacket::getNbrModules() const
{
   return readLong( HEADER_SIZE + 4);
}

ShutdownPacket::ShutdownPacket()
   : Packet( HEADER_SIZE, Packet::m_nbrPriorities - 1, 
             PACKETTYPE_SHUTDOWN, 0, 0)
{
}
