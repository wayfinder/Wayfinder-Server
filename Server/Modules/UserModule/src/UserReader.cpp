/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Queue.h"
#include "UserReader.h"
#include "PeriodicPacket.h"
#include "SimpleBalancer.h"
#include "ModuleList.h"
#include "UserProcessor.h"
#include "StatisticsPacket.h"

/**
 * The UserReader's balancer is only used when sending requests which
 * can be handles by both leader and availables. This balancer prefers
 * to send packets to availables rather than handling them in the leader
 * (since we want to keep the leader free for the requests only the 
 * leader can handle).
 */
class UserBalancer : public SimpleBalancer {
public:
   UserBalancer( const IPnPort& ownAddr,
                 const MC2String& moduleName,
                 StatisticsPacket* ownStatistics ) 
   : SimpleBalancer( ownAddr, moduleName, ownStatistics, false ) {
   }
   
   /**
    * Reimplemented from SimpleBalancer.
    */
   virtual bool getModulePackets( PacketSendList& packets,
                                  RequestPacket* request ) {
      ModuleNotice* mn = getBestModule();
      if( mn != NULL ) {
         mn->addPacketToQueue( MAX_UINT32 );
         packets.push_back( make_pair( IPnPort( mn->getIP(), mn->getPort() ),
                                       request ) );
      }
      return true;      
   }
   
private:
   
   /**
    * This function will find the most appropriate module to send the
    * request to. With the current implementation this means first choosing
    * the module the least amount of requests left to process, if several
    * modules have the same amount of work to do, availables are preferred,
    * and among availables the one with least load is preferred.
    */
   ModuleNotice* getBestModule() {
      ModuleNotice* best = NULL;
      uint32 bestValue   = MAX_UINT32;
      float bestLoad     = 10000;

      for ( ModuleList::iterator it = m_moduleList->begin();
            it != m_moduleList->end();
            ++it ) {
         ModuleNotice* mn = *it;
         float load = mn->getLoad1();
         if ( mn->isLeader() ) {
            load = 10000;
         }

         // Take shortest queue. 
         uint32 value = mn->getPacketsInQueue() + mn->getRequests();

         if ( ( value < bestValue ) ||
               // If queues are the same take least load.
               ( ( value == bestValue ) && ( load < bestLoad ) ) ) {
            bestValue = value;
            best      = mn;
            bestLoad  = load;
         }
      }
      return best;
   }
};

UserReader::UserReader(Queue *jobQueue,
                       NetPacketQueue& sendQueue,
                       MapSafeVector* loadedMaps,
                       PacketReaderThread* packetReader,
                       uint16 port,
                       uint32 definedRank )
      : StandardReader(MODULE_TYPE_USER,
               jobQueue, sendQueue,
               loadedMaps,
               *packetReader,
               port,
               definedRank,
               false )
{
   delete m_balancer;
   std::auto_ptr<StatisticsPacket> statPacket( createStatisticsPacket() );
   m_balancer = new UserBalancer( m_ownAddr,
                                  getName(),
                                  statPacket.get() );
}


UserReader::~UserReader() {
}


bool 
UserReader::leaderProcessCtrlPacket(Packet *p) {
   if (! StandardReader::leaderProcessCtrlPacket(p)) {
      mc2dbg2 << "UserReader::leaderProcessCtrlPacket( "
                  << p->getSubTypeAsString() << " )"<< endl;
      /// XXX: Should have a list of databases to write to, but if only 
      /// one this works fine!
      switch( (int) p->getSubType() ) {
         case Packet::PACKETTYPE_USERADDREQUEST :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;
         case Packet::PACKETTYPE_USERDELETEREQUEST :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;
            
         case Packet::PACKETTYPE_USERCHANGEREQUEST :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;
            
         case Packet::PACKETTYPE_ADDCELLULARPHONEMODELREQUESTPACKET :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;
            
         case Packet::PACKETTYPE_CHANGEUSERPASSWORDREQUESTPACKET :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;

         case Packet::PACKETTYPE_CHANGECELLULARPHONEMODELREQUESTPACKET :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;

         case Packet::PACKETTYPE_LISTDEBITREQUESTPACKET :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;

         case Packet::PACKETTYPE_ADDUSERNAVDESTINATIONREQUESTPACKET :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;

         case Packet::PACKETTYPE_DELETEUSERNAVDESTINATIONREQUESTPACKET :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;

         case Packet::PACKETTYPE_CHANGEUSERNAVDESTINATIONREQUESTPACKET :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;

         case Packet::PACKETTYPE_ROUTESTORAGE_ADD_ROUTE_REQUEST :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;

         case Packet::PACKETTYPE_USER_CREATE_SESSION_REQUEST :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;
            
         case Packet::PACKETTYPE_ROUTESTORAGE_CHANGE_ROUTE_REQUEST :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break; 

         case Packet::PACKETTYPE_TRANSACTION_REQUEST :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;

         case Packet::PACKETTYPE_TRANSACTION_DAYS_REQUEST :
            sendAndDeletePacket( p, m_ownAddr );
            return true;
            break;
      };
      
      // Not handled
      return false;
   } else {
      // The packet was handled by the method in the superclass
      return true;
   }
}

void
UserReader::sendRequestPacket(RequestPacket* request) {
   // Any module may process requests which only causes reads from the
   // database, so let those requests be handled by the base class
   // which will let the balancer decide which module to use.
   if ( UserProcessor::readOnly( request ) ) {
      StandardReader::sendRequestPacket( request );
   } else {
      // These packets must be handled in the leader since they can cause
      // writes to the database.
      sendAndDeletePacket( request, m_ownAddr );
   }
}


void 
UserReader::becomeLeader() {
   StandardReader::becomeLeader();
   
   // Will cause UserProcessor to do a database check the first time
   sendPeriodicToMyself();
}

void
UserReader::periodicMethod() {
   // Delayed debits check
   sendPeriodicToMyself();   
}

void 
UserReader::sendPeriodicToMyself() {
   sendAndDeletePacket( new PeriodicRequestPacket(), m_ownAddr );
}
