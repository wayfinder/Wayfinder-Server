/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SupervisorReceiver.h"
#include "ISABThread.h"
#include "DatagramSocket.h"
#include "PacketReceiver.h"
#include "Properties.h"
#include "SupervisorStorage.h"

SupervisorReceiver::SupervisorReceiver() 
      : m_newAllowedMapSets( MAX_UINT32 ), m_allowedMapSets( MAX_UINT32 ),
        m_monitor( new ISABMonitor() ), m_receiver( NULL ), 
        m_nbrReceivers( 0 ), m_packetReceiver( NULL )
{}


SupervisorReceiver::~SupervisorReceiver() {
   cleanup();
}


void 
SupervisorReceiver::cleanup() {
   delete m_packetReceiver;
   m_packetReceiver = NULL;
   if ( m_receiver != NULL ) {
      for( uint32 i = 0; i < m_nbrReceivers; i++) {
         delete m_receiver[ i ];
      }
   }
   delete [] m_receiver;
   m_receiver = NULL;
}


Packet* 
SupervisorReceiver::receiveAndCreatePacket( uint32 timeOut, 
                                            DatagramReceiver*& sock )
{
   if ( m_newAllowedMapSets != m_allowedMapSets ) {
      cleanup();
      m_allowedMapSets = m_newAllowedMapSets;
   }

   if ( m_receiver == NULL ) {
      // Make new
      uint32 mapSetCount = 1;
      if ( Properties::getProperty( "MAP_SET_COUNT" ) != NULL ) {
         mapSetCount = Properties::getUint32Property( "MAP_SET_COUNT", 1 );
      }

      uint32 nbrModules = 0;
      // Count module
      while ( SupervisorStorage::modules[nbrModules] != 
              MODULE_TYPE_INVALID ) 
      {
         nbrModules++;
      }

      // Double the number of receivers to receive from leaders and avails.
      m_nbrReceivers = nbrModules * mapSetCount * 2;
      m_receiver = new DatagramReceiver*[ m_nbrReceivers ];
      // Join the groups that we want to listen to.
   
      for ( uint32 i = 0 ; i < nbrModules ; ++i ) {
         for ( int leader = 0; leader < 2; ++leader ) {
            uint32 ip =
               MultiCastProperties::getNumericIP( 
                  SupervisorStorage::modules[ i ], leader );
            uint16 port = 
               MultiCastProperties::getPort( SupervisorStorage::modules[i],
                                             leader );
            
            mc2dbg4 << "[Supervisor]: ip = " << ip << " port = "
                    << port << endl;
         
            for ( uint32 m = 0 ; m < mapSetCount ; ++m ) {
               if ( (1<<m) & m_allowedMapSets ) {
                  uint32 mip = ip + (m << 8);
                  uint16 mport = port | (m << 13);
                  m_receiver[ (2*i + leader)*mapSetCount + m ]
                     = new DatagramReceiver( 
                        mport, DatagramReceiver::REUSEADDR );
                  m_receiver[ (2*i + leader)*mapSetCount + m ]
                     ->joinGroup( mip );
               } else {
                  m_receiver[ (2*i + leader)*mapSetCount + m ] = NULL;
               }
            }
         }
      }

      // Create a packetreceiver and add the receivers to it.
      m_packetReceiver = new PacketReceiver();
      
      for ( uint32 i = 0 ; i < m_nbrReceivers ; i++ ) {
         if ( m_receiver[ i ] != NULL ) {
            m_packetReceiver->addDatagramSocket( m_receiver[ i ] );
         }
      }
   } // End if create new


   Packet* packet = m_packetReceiver->receiveAndCreatePacket( timeOut );
   void* received = m_packetReceiver->getSocketThatReceived();

   if ( packet != NULL && 
        (packet->getSubType() == Packet::PACKETTYPE_STATISTICS 
         || packet->getSubType() == Packet::PACKETTYPE_HEARTBEAT) ) 
   {
      uint32 i = 0;
      sock = m_receiver[ 0 ];
      while ( i < m_nbrReceivers && sock != received ) {
         sock = m_receiver[ i ];
         i++;
      }
      if ( sock != received ) {
         sock = NULL;
      }
   } // else irrelevant packet or timeout      

   return packet;
}

