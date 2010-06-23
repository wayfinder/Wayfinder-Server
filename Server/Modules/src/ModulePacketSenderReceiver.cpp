/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ModulePacketSenderReceiver.h"
#include "DatagramSocket.h"
#include "Properties.h"
#include "multicast.h"

namespace {
// just helper to create leader/avail sockets
void setupDatagram( auto_ptr<Socket>& socket,
                    const IPnPort& addr ) {
   DatagramReceiver* datagram = 
   new DatagramReceiver( addr.getPort(),
                         DatagramReceiver::REUSEADDR );
   if ( ! datagram->joinGroup( addr.getIP() ) ) {
      mc2log << fatal << "Module PSR could not join group" << endl;
      MC2_ASSERT( false );
   }
   socket.reset( datagram );
}
}

ModulePacketSenderReceiver::
ModulePacketSenderReceiver( uint16 listenPort,
                            const IPnPort& leaderAddr, 
                            const IPnPort& availableAddr ):
   PacketSenderReceiver( listenPort ),
   m_newState( AVAILABLE ),
   m_changeState( true ),
   m_leaderAddr( leaderAddr ),
   m_availableAddr( availableAddr )
{
   if ( Properties::getMapSet() != MAX_UINT32 ) {
      m_leaderAddr = MultiCastProperties::changeMapSetAddr( leaderAddr );
      m_availableAddr = MultiCastProperties::changeMapSetAddr( availableAddr );
   }

   setupDatagram( m_availListenSocket, m_availableAddr );
   // always listen on avail socket
   addPermanentSelectable( *m_availListenSocket, PacketSenderReceiver::READ );

   setupDatagram( m_leaderListenSocket, m_leaderAddr );
   addPermanentSelectable( *m_leaderListenSocket, PacketSenderReceiver::READ );

   mc2dbg << "[MPSR]: leaderAddr: " << m_leaderAddr 
          << ", availAddr: " << m_availableAddr << endl;
}

ModulePacketSenderReceiver::~ModulePacketSenderReceiver() 
{
   if ( m_availListenSocket.get() ) {
      removePermanentSelectable( *m_availListenSocket );
   }
   if ( m_leaderListenSocket.get() ) {
      removePermanentSelectable( *m_leaderListenSocket );
   }
}

void ModulePacketSenderReceiver::selectDone() 
{
   ISABSync sync( m_stateLock );
   // now we can safetly update the state, if needed
   if ( ! m_changeState ) {
      return;
   }

   clearIdleAndCachedConnections();

   removePermanentSelectable( *m_leaderListenSocket );
   removePermanentSelectable( *m_availListenSocket );

   mc2dbg << "[MPSR] Changing state to ";

   const int iotype = PacketSenderReceiver::READ;

   switch ( m_newState ) {
   case LEADER:
      mc2dbg << "LEADER" << endl;
      
      dynamic_cast<DatagramReceiver&>( *m_leaderListenSocket ).clear();

      addPermanentSelectable( *m_leaderListenSocket, iotype );
      addPermanentSelectable( *m_availListenSocket, iotype );
      break;
   case AVAILABLE:
      mc2dbg << "AVAILABLE" << endl;
     
      dynamic_cast<DatagramReceiver&>( *m_availListenSocket ).clear();

      addPermanentSelectable( *m_availListenSocket, iotype );
      // By adding it without any iotype (0) we keep it
      // in the permanent list and remove it from the selectable list
      // in the selector
      addPermanentSelectable( *m_leaderListenSocket, 0 );
      break;
   }

   m_changeState = false;
}

void ModulePacketSenderReceiver::becomeLeader() {
   ISABSync sync( m_stateLock );
   m_changeState = true;
   m_newState = LEADER;
}

void ModulePacketSenderReceiver::becomeAvailable() {
   ISABSync sync( m_stateLock );
   m_changeState = true;
   m_newState = AVAILABLE;
}


Socket& ModulePacketSenderReceiver::getLeaderListenSocket() 
{
   return *m_leaderListenSocket;
}

Socket& ModulePacketSenderReceiver::getAvailListenSocket() 
{
   return *m_availListenSocket;
}

