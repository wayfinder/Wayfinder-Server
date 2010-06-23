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

#include "PacketResendHandler.h"

#include "SystemPackets.h"
#include "LeaderIPPacket.h"
#include "DatagramSocket.h"
#include "NetUtility.h"
#include "TimeUtility.h"
#include "IPnPort.h"
#include "QueuedPacketSender.h"
#include "PacketSenderReceiver.h"
#include "PacketContainerTree.h"
#include "PacketDump.h"
#include "StringTable.h"

class ServerPacketSender: public QueuedPacketSender {
public:
   ServerPacketSender( PacketSenderReceiver& psr,
                       uint32 tcpLimit ):
      QueuedPacketSender( psr.getSendQueue() ),
      m_tcpLimit ( tcpLimit ),
      m_origin( psr.getAddr() ),
      m_senderReceiver( psr )
   { }

   bool sendPacket( Packet* pack, const IPnPort& destination ) {
      sendUDP( pack, destination );
      return true;
   }

   bool send( PacketContainer& cont ) {

      cont.getPacket()->setOriginAddr( m_origin );
      // set timeout, convert from ms to sec, round up
      cont.getPacket()->
         setTimeout( ( 999 + cont.getNextResendTimeout() ) / 1000  );

      // Set destination, i.e. the multicast address of the leader
      // of the correct module type and mapSet.
      IPnPort dest;
      cont.getIPAndPort( dest.first, dest.second );

      if ( cont.getPacket()->getLength() < m_tcpLimit ) {
         IPnPort tcpDest = getLeaderAddr( dest );
         if ( tcpDest.getIP() != 0 ) {
            sendTCP( cont.getPacket()->getClone(), tcpDest );
            return true;
         }

         return sendPacket( cont.getPacket()->getClone(), dest );
      } else {

         LeaderIPRequestPacket* req = 
            new LeaderIPRequestPacket( cont.getModuleType(),
                                       static_cast< RequestPacket& >
                                       ( *cont.getPacket() ),
                                       dest );
         req->setOriginAddr( m_origin );
         IPnPort tcpDest = getLeaderAddr( dest );
         if ( tcpDest.getIP() != 0 ) {
            mc2dbg2 << "Sending leader ip as tcp" << endl;
            mc2dbg2 << "to destination: " << tcpDest << endl;

            sendTCP( req, tcpDest );
            return true;
         }

         mc2dbg2 << "Sending leader ip as udp" << endl;
         mc2dbg2 << "to destination: " << dest << endl;
         return sendPacket( req, dest );
      }

      return true;
   }

   void sendTCP( Packet* pack, const IPnPort& destination ) {
      QueuedPacketSender::sendTCP( pack, destination );
   }

   void updateLeaderAddr( const LeaderIPReplyPacket& leaderPack ) {
      // If we sent this packet to a multicast address, then save the real 
      // IPnPort for nice TCP sending in the Something::send

      if ( !NetUtility::
           isMulticast( leaderPack.getOriginalDestAddr().getIP() ) ) {
         return;
      }
      m_leaderMap[ leaderPack.getOriginalDestAddr() ] = 
         leaderPack.getLeaderAddr();
   }

   IPnPort getLeaderAddr( const IPnPort& addr ) const {
      map<IPnPort, IPnPort>::const_iterator it = m_leaderMap.find( addr );
      if ( it == m_leaderMap.end() ) { 
         return IPnPort();
      }
      if ( m_senderReceiver.hasCachedConnection( (*it).second ) ) {
         return (*it).second;
      }

      return IPnPort();
   }

private:
   uint32 m_tcpLimit;
   IPnPort m_origin;

   map<IPnPort, IPnPort> m_leaderMap;
   const PacketSenderReceiver& m_senderReceiver;
};

PacketResendHandler::PacketResendHandler( uint16 port ):
   m_contTree( new PacketContainerTree() ),
   m_answerTree( new PacketContainerTree() ),
   m_running( true ),
   m_receiveThread( new ReceiveThread( this ) ),
   m_timeoutThread( new TimeoutThread( this ) ),
   m_senderReceiver( new PacketSenderReceiver( port ) ),
   m_packetSender( new
                   ::ServerPacketSender( *m_senderReceiver,
                                         Packet::getTCPLimitSize() ) )
{

}


PacketResendHandler::~PacketResendHandler() {
   m_running = false;

   ISABThreadHandle receiveHandle = m_receiveThread;
   ISABThreadHandle timeoutHandle = m_timeoutThread;
   m_receiveThread->terminate();
   m_timeoutThread->terminate();
   // wake up the receivethread
   m_senderReceiver->getReceiveQueue().enqueue( NULL );

   notifyAnswer();
   ISABSync* sync = new ISABSync( m_monitor );
   m_monitor.notifyAll();
   delete sync;
   // Wait for ReceiveThread and TimeoutThread to finnish so that
   // we can delete stuff
   while ( receiveHandle->isAlive() || timeoutHandle->isAlive() ) {
      ISABThread::yield();
   }
}

Packet*
PacketResendHandler::receive(uint32 maxWaitTime)
{
   return m_senderReceiver->getReceiveQueue().dequeue( maxWaitTime );
}

void 
PacketResendHandler::start() {
  // Start threads

   m_senderReceiver->start();
   m_timeoutThread->start();
   m_receiveThread->start();
}

void
PacketResendHandler::addAndSend( PacketContainer* cont ) {
   ISABSync sync( m_monitor );
   cont->setServerTimestamp( TimeUtility::getCurrentTime() );
   m_contTree->add( cont );
   send( cont );
   m_monitor.notifyAll();
}


PacketContainer*
PacketResendHandler::getAnswer() {

   ISABSync sync( m_answerMonitor );
   if ( m_answerTree->getCardinal() == 0 ) {
      try {
         m_answerMonitor.wait();
      }
      catch(const JTCInterruptedException &) {
      }
   }

   if ( m_answerTree->getCardinal() > 0 ) {
      PacketContainer* pc = m_answerTree->getMin();
      if ( pc != NULL ) {
         m_answerTree->remove( pc );
         return pc;
      } else {
         return NULL;
      }
   } else {
      return NULL;
   }
}


void
PacketResendHandler::notifyAnswer() {
   ISABSync sync( m_answerMonitor );

   m_answerMonitor.notifyAll();   
}


PacketContainer*
PacketResendHandler::getAnswerNow() {
   ISABSync sync( m_answerMonitor );
   if ( m_answerTree->getCardinal() > 0 ) {
      PacketContainer* pc = m_answerTree->getMin();
      if ( pc != NULL ) {
         m_answerTree->remove( pc );
         return pc;
      } else {
         return NULL;
      }
   } else {
      return NULL;
   }
}


uint16
PacketResendHandler::getPort() {
   return m_senderReceiver->getAddr().getPort();
}

PacketContainer* 
PacketResendHandler::checkAndResend() {
   ISABSync sync( m_monitor );
   
   mc2dbg4 << "PacketResendHandler::checkAndResend()" << endl;
   
   while ( m_contTree->empty() && m_running ) {
      try {
         m_monitor.wait();
      }
      catch ( const JTCInterruptedException & ) {}
   }

   if ( !m_running ) {
      mc2dbg4 << "PacketResendHandler::checkAndResend() not running" 
              << endl;
      return NULL;
   }   

   mc2dbg4 << "PacketResendHandler::checkAndResend() checking resend" 
           << endl;
      
   uint32 timestamp = TimeUtility::getCurrentTime();
   PacketContainer* cont = m_contTree->getMin();
   
   if ( ( (timestamp - cont->getServerTimestamp() ) 
          >= cont->getNextResendTimeout() ) ) 
   { // The packet is too old now - resend
      if ( cont->getServerResend() < cont->getNbrResend() ) {
         cont->setServerTimestamp(timestamp);
         cont->setServerResend( cont->getServerResend() + 1 );
         // Handle partially received packets
         cont->resetAnswerData();
         mc2dbg4 << "Resending req: " 
                 << cont->getPacket()->getRequestID() << " PackID: " 
                 << cont->getPacket()->getPacketID() << endl;
         send( cont ); // Send the packet
         // Put cont in right order of tree
         m_contTree->remove( cont ); // Remove the packetcontainer from tree
         m_contTree->add( cont ); // Put the packet last in tree.
         return NULL;
      } else {
         m_contTree->remove( cont );
         mc2dbg4 << "Maximum resend count (" << cont->getNbrResend()
                 << ") - will not resend" 
                 << " ReqID: " << cont->getPacket()->getRequestID() 
                 << " PackID: " << cont->getPacket()->getPacketID() 
                 << endl;
         return cont; // Return requestPacket
      }
   } else { // No need to resend the packet yet
      mc2dbg4 << "Not time to resend waiting: " << endl
              << "cont->getServerTimestamp(): " 
              << cont->getServerTimestamp()
              << ", timestamp: " << timestamp
              << ", cont->getNextResendTimeout(): " 
              << cont->getNextResendTimeout()
              << endl;
      try {
         int32 waitUntilThis = 
            cont->getServerTimestamp() +  cont->getNextResendTimeout();
         int32 timeLeft = waitUntilThis - timestamp;
         if ( timeLeft < 0 ) {
            mc2log << "[PRH]: Negative wait time" << endl;
         } else {
            m_monitor.wait( timeLeft );
         }
      }
      catch ( const JTCInterruptedException & ) {}
   }
   return NULL;
}

bool 
PacketResendHandler::findAndRemoveIfDone( Packet* pack, 
                                          PacketContainer*& res ) 
{
   ISABSync sync( m_monitor );

   int pid = pack->getPacketID();
   int rid = pack->getRequestID();
   PacketContainer *c = m_contTree->linearGetWithRequestAndPackID( rid,
                                                                  pid );
   if ( c != NULL) {
      c->addAnswer( pack );
      if ( c->answerComplete() ) {
         m_contTree->remove( c );
         res = c;
      } else {
         res = NULL;
      }
      return true;
   } else {
      mc2dbg2 << "PacketResendHandler::findAndRemoveIfDone packet( " 
              << pack->getPacketID() << ", " << pack->getRequestID() << " )"
              << " Not found" << endl;
      DEBUG2( PacketUtils::dumpHeaderToFile( stdout, *pack ) );
      delete pack;
      res = NULL;
      return false; 
   }
}


bool 
PacketResendHandler::upDateRequestPacket( Packet* pack, 
                                          PacketContainer*& res )
{
   ISABSync sync( m_monitor );

   bool ok = true;

   int pid = pack->getPacketID();
   int rid = pack->getRequestID();
	PacketContainer *c = 
           m_contTree->linearGetWithRequestAndPackID( rid, pid );
   if ( c != NULL) {
      AcknowledgeRequestReplyPacket* a = 
         static_cast<AcknowledgeRequestReplyPacket*>( pack );
      if ( a->getStatus() == StringTable::OK ) {
         c->setResendTimeout( a->getETA() );
         m_contTree->remove( c ); // Remove the packetcontainer from tree
         m_contTree->add( c ); // Put the packet back in tree.
         m_monitor.notifyAll(); // New resend timeout
      } else if ( a->getStatus() == StringTable::ERROR_MAP_LOADED ) {
         // Not error, but map loaded.
         send(c);
      } else {
         // Return false and c
         m_contTree->remove( c );
         res = c;
         ok = false;
      }
   }

   return ok;
}

void 
PacketResendHandler::addAnswer( PacketContainer* cont ) {
   ISABSync sync( m_answerMonitor );
   
   m_answerTree->add( cont );
   m_answerMonitor.notifyAll();
}


bool
PacketResendHandler::send( PacketContainer* cont ) {
   return m_packetSender->send( *cont );
}


void 
PacketResendHandler::sendTCPPackets( Packet* p )
{

   ISABSync sync( m_monitor );

   PacketContainer *cont = 
      m_contTree->linearGetWithRequestAndPackID( p->getRequestID(),
                                                 p->getPacketID() );
   if ( cont == NULL ) {
      mc2dbg4 << "Packet not in container." << endl;
      delete p;
      return;
   }

   MC2_ASSERT( cont->getPacket() );
  
   LeaderIPReplyPacket* leaderIP = static_cast<LeaderIPReplyPacket*>( p );

   mc2dbg2 << "[PRH] Got leader ip: " << leaderIP->getLeaderAddr() << endl;

   m_packetSender->updateLeaderAddr( *leaderIP );
   
   Packet* pack = cont->getPacket()->getClone();

   uint32 ip = leaderIP->getIP();
   uint16 port = leaderIP->getPort();
   m_packetSender->sendTCP( pack, IPnPort( ip, port ) );

   delete p;
}


// **********************************************************************
// Receive Thread
// **********************************************************************


PacketResendHandler::
ReceiveThread::ReceiveThread( PacketResendHandler* handler ):
   ISABThread( NULL, "PacketResendHandler::ReceiveThread" ),
   m_handler( handler )
{

}


PacketResendHandler::ReceiveThread::~ReceiveThread() {
}


void 
PacketResendHandler::ReceiveThread::run() {
   Packet* pack;
   PacketContainer* cont;
   
   const uint32 maxWaitTime = 1000000;
      
   while ( !terminated ) {
      
      pack = m_handler->receive(maxWaitTime);      
      
      if ( pack == NULL ) {
         continue;
      }

      mc2dbg8 << "ReceiveThread::run got Packet!" << endl;

      switch ( pack->getSubType() ) {
      case Packet::PACKETTYPE_ACKNOWLEDGE:
         // INFO: All systempackets here
         if ( ! m_handler->upDateRequestPacket( pack, cont ) ) {
            // Return request!
            if ( cont != NULL ) {
               m_handler->addAnswer( cont );
            }
         } // Else timeout updated

         delete pack;

         break;
      case Packet::PACKETTYPE_LEADERIP_REPLY:
         mc2dbg4 << "[PRH]: PACKET_TYPELEADERIP_REPLY" << endl;
         m_handler->sendTCPPackets( pack );
         break;
      default:
         if ( m_handler->findAndRemoveIfDone( pack, cont ) ) {
            if ( cont != NULL ) {
               mc2dbg4 << "ReceiveThread Removed packet from list,"
                       << " ReqID: " 
                       << cont->getPacket()->getRequestID() 
                       << ", PacketID: " 
                       << cont->getPacket()->getPacketID() << endl;
               m_handler->addAnswer( cont );
            }
         }
         break;
      }

   }

   mc2dbg << "PacketResendHandler::ReceiveThread::run ends" << endl;
}


// **********************************************************************
// Timeout Thread
// **********************************************************************


PacketResendHandler::
TimeoutThread::TimeoutThread( PacketResendHandler* handler ):
   ISABThread( NULL, "PacketResendHandler::TimeoutThread" ),
   m_handler( handler )
{

}


PacketResendHandler::TimeoutThread::~TimeoutThread() {
}


void PacketResendHandler::TimeoutThread::run()
{
   PacketContainer* cont = NULL;
   while( !terminated ) {
      // Wait for new packets and resend if necessary
      if ( (cont = m_handler->checkAndResend()) != NULL ) {
         // Max count reached - return request packet 
         m_handler->addAnswer( cont );
      } else {
         mc2dbg4 << "TimeoutThread::run() no timeoutpacket" << endl;
      }
   } 
   mc2dbg << "[PacketResendHandler]::TimeoutThread::run ends" << endl;
}

