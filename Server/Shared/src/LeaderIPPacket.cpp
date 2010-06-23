/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "LeaderIPPacket.h"
#include "StringTable.h"

// --------------------------------------------------------------
// ----------------------------------- LeaderIPRequestPacket ----

namespace {
inline void writeAddr( Packet& p, int& startAddr, const IPnPort& addr ) {
   p.incWriteLong( startAddr, addr.getIP() );
   p.incWriteLong( startAddr, addr.getPort() );
}

inline IPnPort getAddr( const Packet& p, int startAddr ) {
   uint32 ip = p.incReadLong( startAddr );
   uint16 port = p.incReadLong( startAddr );
   return IPnPort( ip, port );
}

}

LeaderIPRequestPacket::LeaderIPRequestPacket( uint16 modType,
                                              const RequestPacket& packet,
                                              const IPnPort& origDestAddr )

  : RequestPacket( REQUEST_HEADER_SIZE + 200,
		   LEADER_IP_REQUEST_PRIO,
		   PACKETTYPE_LEADERIP_REQUEST,
		   packet.getPacketID(),
		   packet.getRequestID(),
		   packet.getMapID() )
{
   int position = REQUEST_HEADER_SIZE;
   incWriteLong( position, modType );
   incWriteLong( position, packet.getSubType() );
   ::writeAddr( *this, position, origDestAddr );;
   setLength( position );
}

bool
LeaderIPRequestPacket::regardingCtrlPacket() const {
   return ( readLong( REQUEST_HEADER_SIZE + 4 ) & CTRL_PACKET_BIT ) != 0;
}

uint16
LeaderIPRequestPacket::getModuleType() const
{
  return readLong( REQUEST_HEADER_SIZE );
}

IPnPort
LeaderIPRequestPacket::getOriginalDestAddr() const 
{
   return ::getAddr( *this, REQUEST_HEADER_SIZE + 8 );
}

// --------------------------------------------------------------
// -------------------------------------- LeaderIPReplyPacket ---

LeaderIPReplyPacket::LeaderIPReplyPacket( const LeaderIPRequestPacket& p,
                                          uint32 ip,
                                          uint16 port,
                                          uint16 modType,
                                          const IPnPort& leaderAddr )
      : ReplyPacket ( REPLY_HEADER_SIZE + 240,
                      PACKETTYPE_LEADERIP_REPLY,
                      static_cast<const RequestPacket*>( &p ),
                      StringTable::OK)

{
  int position = REPLY_HEADER_SIZE;
  incWriteLong( position, ip );
  incWriteLong( position, port );
  incWriteLong( position, modType );
  incWriteLong( position, p.getMapID() );
  ::writeAddr( *this, position, leaderAddr );
  ::writeAddr( *this, position, p.getOriginalDestAddr() );

  setLength( position );
}


uint32 
LeaderIPReplyPacket::getIP() const
{
  int position = REPLY_HEADER_SIZE;
  return uint32( readLong( position ) );
}

uint16 
LeaderIPReplyPacket::getPort() const 
{
  int position = REPLY_HEADER_SIZE + 4;
  return uint16( readLong( position ) );
}


uint16
LeaderIPReplyPacket::getModuleType() const
{
  int position = REPLY_HEADER_SIZE + 8;
  return uint16( readLong( position ) );
}
  
uint32
LeaderIPReplyPacket::getMapID()  const
{
   int position = REPLY_HEADER_SIZE + 12;
   return readLong( position );
}

IPnPort
LeaderIPReplyPacket::getLeaderAddr() const 
{
   return ::getAddr( *this, REPLY_HEADER_SIZE + 16 );
}

IPnPort
LeaderIPReplyPacket::getOriginalDestAddr() const 
{
   return ::getAddr( *this, REQUEST_HEADER_SIZE + 24 );
}
