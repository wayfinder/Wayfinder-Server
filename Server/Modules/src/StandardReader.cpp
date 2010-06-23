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


#include "StandardReader.h"

#include "MapSafeVector.h" 

#include "Fifo.h"
#include "LeaderIPPacket.h"
#include "MapNotice.h"
#include "DatagramSocket.h"
#include "PacketReaderThread.h"

#include "AllMapPacket.h"
#include "LoadMapPacket.h"

#include "ModulePacket.h"
#include "StatisticsPacket.h"

#include "Properties.h"
#include "PropertyHelper.h"
#include "multicast.h"
#include "PacketQueue.h"
#include "TestPacket.h"
#include "AlivePacket.h"

// For push (i.e. receiving push)
#include "TCPSocket.h"
#include "ModulePushList.h"
#include "PushServices.h"
#include "SubscriptionResource.h"
#include "PacketContainer.h"

#include "Balancer.h"
#include "SimpleBalancer.h"
#include "PushService.h"
#include "NetUtility.h"
#include "TimeUtility.h"
#include "LeaderStatus.h"
#include "LogBuffer.h"
#include "MapBits.h"
#include "DefaultJobTimeoutHandler.h"

#include <memory>
#include <vector>


// NB! This must be 3 in the "ordinary" case, but when creating maps we 
//     must increase it :(
// The cost of not having a map loaded in any module.
#define MIN_WAIT_NBR_HEARTBEATS 5 




// Define LEADER_IS_IN_THE_MODULE_LIST if the leader is in the modulelist
// The value ZERO_AVAILABLES is used when checking if a leader should
// resign when hearing another leader's heartbeat.
// A leader with zero availables will resign when hearing a leader with
// more than zero availables.
#define LEADER_IS_IN_THE_MODULE_LIST
#ifdef LEADER_IS_IN_THE_MODULE_LIST
#define ZERO_AVAILABLES 1
#else
#define ZERO_AVAILABLES 0
#endif

StandardReader::StandardReader(moduletype_t moduleType,
               Queue *jobQueue, 
               NetPacketQueue& sendQueue,
               MapSafeVector* p_loadedMaps,
               PacketReaderThread& packetReader,
               uint16 listenPort,
               int32 definedRank,
               bool usesMaps,
               vector<PushService*>* wantedServices,
               bool waitForPush) 
      : ReaderBase( jobQueue, 
                    sendQueue,
                    p_loadedMaps,
                    IPnPort( NetUtility::getLocalIP(),
                             listenPort ),
                    IPnPort( packetReader.getLeaderIP(),
                             packetReader.getLeaderPort() ),
                    IPnPort( packetReader.getAvailableIP(),
                             packetReader.getAvailablePort() ),

                    ( (wantedServices != NULL) && waitForPush) ),
        m_inPacketQueue( packetReader.getPacketQueue() ),
        m_packetReader( packetReader ),
        m_startAsLeader( false ),
        m_leaderStatus( new LeaderStatus() ),
        m_moduleType( moduleType ),
        m_timeoutHandler( new DefaultJobTimeoutHandler() )
{
   mc2dbg << "defined rank = " << definedRank << endl;
   mc2dbg << "own port: " << listenPort << endl;

   setModuleName();

   readPropValues();

   std::auto_ptr<StatisticsPacket> statPacket( createStatisticsPacket() );
   m_balancer = new SimpleBalancer(m_ownAddr, getName(), 
                                   statPacket.get(), usesMaps);

   mc2dbg << "[StandardReader]: Packet::TCP_LIMIT_SIZE = "
          << Packet::getTCPLimitSize() << endl;
#ifdef SINGLE_VERSION
   mc2log << info << "StandardReader::StandardReader(): Module running in SINGLE mode" << endl;
#endif

   p_loadedMaps->setAddr( m_ownAddr );

   m_loadedMaps = p_loadedMaps;
   
   m_definedRank = definedRank;
   m_internalStatus = NO_VOTING; // Assume no voting when starting
   m_nbrHeartBeat    = 0;
   
   if ( m_startAsLeader ) {
      m_leaderStatus->becomeLeader();
   } else {
      m_leaderStatus->becomeAvailable();
   }

   m_nbrOfReq        = 0;
   m_nbrVotes        = 0;
   
   m_pushList       = NULL;
   m_pushServices   = NULL;
   m_wantedServices = NULL;

   // FIXME: Move to function
   if ( wantedServices != NULL && ! wantedServices->empty() ) {
      m_pushServices = new PushServices;

      PacketContainerList dummyPackets;
      vector<uint32>      dummyTimes;
      
      for( vector<PushService*>::iterator it = wantedServices->begin();
           it != wantedServices->end();
           ++it) {
         m_pushServices->addPushService((*it)->clone(),
                                        dummyPackets,
                                        dummyTimes);
      }
      mc2dbg << "[StandardReader] size of packetlist is " << dummyPackets.size()
             << endl;
      // Create push list so that we can receive push packets and sockets.
      m_pushList = new ModulePushList();

      //      m_packetReader.setPushList(m_pushList);

      m_wantedServices = wantedServices;
      // Ugly. We need this to wake up the reader when a
      // new map has been loaded.
      m_loadedMaps->setReaderFifo(&m_inPacketQueue);
   }
}


StandardReader::~StandardReader() {
   
   delete m_balancer;
}


bool
StandardReader::isLeader() const
{
   return m_leaderStatus->isLeader();
}

const LeaderStatus*
StandardReader::getLeaderStatus() const {
   return m_leaderStatus.get();
}

uint32
StandardReader::getNbrVotes()
{
   return m_nbrVotes;
}

uint32
StandardReader::getNbrRequests()
{
   return m_nbrOfReq;
}

int32
StandardReader::getRank()
{
   return m_definedRank;
}

void
StandardReader::setRank(int32 newRank)
{
   m_definedRank = newRank;
}

int
StandardReader::sendPushPackets(PacketContainerList& packets)
{
   int sentPackets = 0;
   // Copied from PushServiceHandler
   while ( !packets.empty() ) {
      PacketContainerList::reference ref = packets.front();
      IPnPort addr;

      ref->getIPAndPort( addr.first, addr.second );
      ref->getPacket()->setOriginIP( m_ownAddr.getIP() );
      ref->getPacket()->setOriginPort( m_packetReader.getPushPort() );
      mc2dbg << "[StandardReader] sendPushPacket to " << addr << endl;
      MC2_ASSERT( ref->getPacket() );
      sendAndDeletePacket( ref->getPacket()->getClone(), addr );
      ++sentPackets;
      delete ref;
      packets.pop_front();      
   }
   return sentPackets;
}

bool
StandardReader::checkPushResources()
{   
   vector<SubscriptionResource*> loaded;
   vector<SubscriptionResource*> deleted;
   m_loadedMaps->getResources(loaded, deleted);

   PacketContainerList packetList;
   vector<uint32> lastUpdateTime;
   // Add the resources to the pushhandler
   // (or remove).
   // We must loop through our subscriptions   
   for( vector<PushService*>::const_iterator it = m_wantedServices->begin();
        it != m_wantedServices->end();
        ++it) {
      // This is no real clone
      PushService* pushService = (*it)->clone();
      uint32 serviceID = pushService->getServiceID();

      mc2dbg4 << "[StandardReader] adding service " << serviceID << endl;
      
      for( vector<SubscriptionResource*>::const_iterator ut = loaded.begin();
           ut != loaded.end();
           ++ut) {
         mc2dbg4 << "[StandardReader] adding resource " << (**ut) << endl;
         pushService->addResource( **ut );         
      }

      for( vector<SubscriptionResource*>::const_iterator ut = deleted.begin();
           ut != deleted.end();
           ++ut) {
         pushService->removeResource( **ut, packetList );
         m_pushServices->removePushServiceResource( serviceID,
                                                    **ut,
                                                    packetList);
      }
      
      m_pushServices->addPushService( pushService,
                                      packetList,
                                      lastUpdateTime);
   }
   mc2dbg4 << "StandardReader::checkPushResources - size of list "
           << packetList.size() << endl;
   if ( sendPushPackets(packetList) > 0 )
      return true;
   else
      return false;
}

bool
StandardReader::handlePush()
{
   mc2dbg8 << "StandardReader::handlePush()" << endl;
   // Check if there is a push list
   if ( m_pushList == NULL ) {
      mc2dbg8 << "StandardReader::handlePush() - m_pushList == NULL" << endl;
      // No push handled.
      return false;
   }
   
   // Check if there was push content.
   auto_ptr<pair<Packet*, TCPSocket*> > packetSocket( m_pushList->remove() );

   bool checkRes = checkPushResources();

   // Check if there was a packet
   if ( packetSocket.get() == NULL ) {
      return checkRes;
   }
   
   // There was stuff in the list.
   PushPacket* packet = (PushPacket*)(packetSocket->first);
   TCPSocket* socket = packetSocket->second;
   PacketContainerList packetList;
   
   // Broken socket
   if ( packet == NULL ) {
      mc2dbg << "StandardReader::handlePush() - socket broken" << endl;
      // Handle broken socket.
      m_pushServices->handleBrokenSocket( socket, packetList);
      // Close and delete the socket.
      delete socket;
      // Send the resulting packets.
      sendPushPackets(packetList);

      return true; // Push data handled
   }
   
   // Socket is not broken.
   bool isDataPacket = false;
   uint32 serviceID = 0;
   SubscriptionResource* resource = NULL;
   m_pushServices->handlePushPacket( packet, socket, packetList,
                                     isDataPacket, serviceID, resource);
   // Send the resulting packets
   sendPushPackets(packetList);
   
   if ( isDataPacket == true ) {
      // Give packet to JobThread
      sendAndDeletePacket(packet, m_ownAddr);
   }

   return true; // Push data handled
}

bool
StandardReader::jobThreadWorkingTooLong()
{
   if ( m_loadedMaps->jobThreadCrashTimeOut() != -1 ) {
      mc2dbg << "[StandardReader] job thread crash timeout reached." << endl;
      m_timeoutHandler->timeout();
      return false;
   }
   
   int res = m_loadedMaps->jobThreadTimeOut();
   if ( res == -1 ) {
      return false;
   }
   if( res != m_currentJobThreadTimeOutJob ) {
      m_currentJobThreadTimeOutJob = res;
      if ( isLeader() ) {
         mc2log << warn << " [StandardReader]: JobThread time out!" << endl;
      } else {
         mc2log << warn << " [StandardReader]: JobThread time out - will hide"
                << endl;
      }
   }
   return true;
}

void StandardReader::handleReceiveTimeout(int32 time1,
                                  int& nbrHeartbeat)
{
   mc2dbg8 << "[StandardReader] handleReceiveTimeout" << endl;
   // Time-out
   if ( isLeader() ) {
      // Time to send heartbeat
      nbrHeartbeat++;
      HeartBeatPacket hp( m_ownAddr, m_definedRank, m_balancer); 
      sendToAvailables( hp );
      // In the future, we should maybe send heartbeats on leaderport
      // too, to check that we listen to the leader port.
      //m_sendSocket->send(&hp, m_leaderIP, m_leaderPort);
      m_lastHeartBeat = TimeUtility::getCurrentTime();

      // Update the balancer
      PacketSendList packets;
      m_balancer->timeout( createStatisticsPacket(), packets );
      sendAndDeletePackets( packets );
      
      // Check that jobthread is still alive.
      if ( jobThreadWorkingTooLong() ) {
         if ( m_balancer->getNbrModules() == ZERO_AVAILABLES ) {
            mc2dbg << "[StandardReader] Zero availables." << endl;
            m_timeoutHandler->timeout();
         } else {
            // FIXME: Set it back if the jobthread wakes up
            setRank(getRank()-10);
            vote();
         }
      }
   } else {
      doVote();
   } // end else (isLeader)
}

void
StandardReader::lastVote() {
   /*
    * This is done directly via DatagramSender because this
    * functionis suppose to be called right before the thread dies
    * which could mean* that the PacketSenderReceiver is also terminating, 
    * so  we can not put anything new in PacketSenderReciever 
    * without the risk of loosing the vote packet.
    */
   DatagramSender sender;
   VotePacket vp(m_ownAddr.getIP(),
                 m_ownAddr.getPort(), m_definedRank, false);
   if ( ! sender.send( &vp, getAvailAddr() ) ) {
      mc2log << error << "[StandardReader] Failed to send vote to " << getAvailAddr() << endl;
   }
   if ( ! sender.send( &vp, getLeaderAddr() ) ) { 
      mc2log << error << "[StandardReader] Failed to send vote to " << getLeaderAddr() << endl;
   }
}

void
StandardReader::vote()
{
   if ( isLeader() )
      becomeAvailable();
   // Turn clock
   m_lastHeartBeat = m_lastHeartBeat -=
      MIN_WAIT_NBR_HEARTBEATS * HEARTBEAT_RATE;
   doVote();
}

void
StandardReader::sendVoteToAll()
{
   VotePacket vp(m_ownAddr.getIP(),
                 m_ownAddr.getPort(), m_definedRank, false); // Not direct
   sendToAvailables( vp );
   sendToLeaders( vp );
}

void StandardReader::doVote()
{
      mc2dbg8 << "[StandardReader] isLeader == FALSE. Socket timed-out,"
              << " m_internalStatus: " << (int) m_internalStatus << endl;
      switch (m_internalStatus) {
         case WON_1 : {
            mc2dbg << "[StandardReader] Sending second VotePacket" << endl;
            m_internalStatus = WON_2;

            sendVoteToAll();
         }
         break;
         case WON_2 : {
            mc2dbg << "[StandardReader] Sending third VotePacket" << endl;
            m_internalStatus = WON_3;

            sendVoteToAll();
         }
         break;
         case WON_3: {
            // I've become leader!!!
            mc2dbg8 << "[StandardReader] I've become leader!" << endl;
            mc2dbg8 << "[StandardReader] Switch listenSocket" << endl;

            // Switch the listen sockets.
            becomeLeader();
            
            HeartBeatPacket hp(m_ownAddr, m_definedRank, 
                               m_balancer);
            sendToAvailables( hp );
            m_lastHeartBeat = TimeUtility::getCurrentTime();
         }
         break;
         default : {
            int timeDiff = TimeUtility::getCurrentTime() - m_lastHeartBeat;
            if ( timeDiff > (HEARTBEAT_RATE * MIN_WAIT_NBR_HEARTBEATS) ) {
               m_nbrVotes++;
               mc2log << info << "[StandardReader] Initiate election!" << endl;
               mc2dbg1 << "[StandardReader] Timed out on socket. No heartbeat from "
                  "leader or vote received, start election!" << endl;
               sendVoteToAll();
               m_internalStatus = WON_1;
            } else {
               mc2dbg << "[StandardReader]: No election yet - timeDiff = "
                      << timeDiff << endl;
            }
         }
      } // end switch
}

void StandardReader::run()
{
   int maxWaitTime = 0;       // Maximum time waiting for message, in ms.
   DEBUG2(int nbrReceived = 0);
   DEBUG2(int milliseconds = TimeUtility::getCurrentTime());

   Packet *packet = NULL;
   m_lastHeartBeat = TimeUtility::getCurrentTime();

   if ( m_startAsLeader ) {
      mc2dbg2 << "[StandardReader] Reader started as leader." << endl;
      becomeLeader();
   } else {
      mc2dbg2 << "[StandardReader] Reader started as available." << endl;
      becomeAvailable();
      // Send statistics to the leader.
      replyToHeartBeat();
   }
  

   // Loop for receiving and distributing messages via the network
   while(!terminated) {
      packet = NULL;

      // Calculate maximum time waiting for message (the time depends on 
      // if running as leader, available or a vote is going on)
      mc2dbg8 << "[StandardReader] status:";
      if ( isLeader() ) {
         maxWaitTime = HEARTBEAT_RATE -
            (TimeUtility::getCurrentTime() - m_lastHeartBeat);
         mc2dbg8 << " isLeader"; 
            
      } else if ( m_internalStatus == WON_1 || 
                  m_internalStatus == WON_2 ||
                  m_internalStatus == WON_3 ) {
         maxWaitTime = ELECTION_WON_RATE;
         mc2dbg8 << " vote";
      } else {
         // Wait until 3 x heartbeat rate
         maxWaitTime = MIN_WAIT_NBR_HEARTBEATS * HEARTBEAT_RATE -
            (TimeUtility::getCurrentTime() - m_lastHeartBeat);
         mc2dbg8 << " available";
      }
      mc2dbg4 << " waitingtime " << maxWaitTime << endl;


      // Wait for packet for maximum maxWaitTime ms.
      int32 time1 = 0;
      time1 = TimeUtility::getCurrentTime();

      mc2dbg4 << "StandardReader: "
              << " queue size in/job: " << m_inPacketQueue.getSize() 
              << "/" << getQueueLength() << endl;

      // Let periodic things happen
      periodicMethod();

      if ( (maxWaitTime > 0) &&
           ((packet = m_inPacketQueue.dequeue(maxWaitTime)) != NULL) ) {

         // Packet recived
         m_nbrOfReq++;

         mc2dbg4 << "[StandardReader] Received packet after " 
                 << ( TimeUtility::getCurrentTime() - time1) << " ms " << endl;
         
         DEBUG2(  if (++nbrReceived % 1000 == 0)
                  {   
                     mc2dbg2 << "[StandardReader] " << nbrReceived 
                             << " packets received in "
                             << (TimeUtility::getCurrentTime()-milliseconds)
                             << " ms." << endl;
                     milliseconds = TimeUtility::getCurrentTime() ;
                  }
         );

         if ( packet->timedOut() ) {
            mc2dbg << "[StandardReader]: Packet timed out, type="
                   << packet->getSubTypeAsString()
                   << " age "
                   << float(packet->getTimeSinceArrival() / 1000.0 )
                   << " max age " << packet->getTimeout() << endl;
            delete packet;
            packet = NULL;
            continue;
         }

         if (packet->getSubType() == Packet::PACKETTYPE_SHUTDOWN) {
            terminated = true;
            sendAndDeletePacket( packet, m_ownAddr );
            continue;
         }

         if ( packet->getOriginPort() == m_ownAddr.getPort() &&
              packet->getOriginIP() == m_ownAddr.getIP() ) {
            switch ( packet->getSubType() ) {
               case Packet::PACKETTYPE_HEARTBEAT:
               case Packet::PACKETTYPE_VOTE:
               case Packet::PACKETTYPE_STATISTICS:
                  break;
               default:
                  mc2dbg8 << "[StandardReader]: Got packet from myself"
                          << endl;
            }
         }
         
         // clear deb info, we use it later to calculate processing time
         packet->setDebInfo(0);
         processPacket(packet);         
      } else {
         // Check for push
         if ( ! handlePush() ) {
            // Handle heartbeat timeout.            
            handleReceiveTimeout(time1, m_nbrHeartBeat);
         }
      } // Endif for receive
   } // end while(!terminated)

   cleanUp();
}

void StandardReader::setJobTimeoutHandler( JobTimeoutHandler* handler ) {
   m_timeoutHandler.reset( handler );
}

bool
StandardReader::readPropValues()
{   
   bool success = true;

   MC2String modulePrefix( StringUtility::copyUpper( getName() ) );

   using PropertyHelper::getMostSpecializedProperty;
   m_maxMem = getMostSpecializedProperty<uint64>( "MODULE_MAX_MEM",
                                                  modulePrefix,
                                                  Properties::getMapSet(),
                                                  30 );
                                                 
   m_optMem = getMostSpecializedProperty<uint64>( "MODULE_OPT_MEM",
                                                  modulePrefix,
                                                  Properties::getMapSet(),
                                                  10 );
   return success;
}

void StandardReader::cleanUp()
{
   delete m_pushList;
   mc2log << info << "[StandardReader] shutdown complete" << endl;
}


void
StandardReader::processPacket(Packet *p)
{
   mc2dbg2 << "StandardReader::processPacket(): " << p->getSubTypeAsString() << endl;
   setLogPrefix(p);

   // Remove ack-packs here so that there will be no problems with
   // them later.
   if ( p->getSubType() == Packet::PACKETTYPE_ACKNOWLEDGE ) {
      mc2dbg << "[StandardReader]: I do not want ack-packs" << endl;
      delete p;
      return;
   }
  
   printDebugPacketInfo(p);

   // Check the packet and find out if ctrl-packet or not
   bool ctrlPacket = (p->getSubType() & CTRL_PACKET_BIT) != 0;

   // Process the packet
   bool packetProcessed = false;

   if ( isLeader() ) {
      // Leader
      mc2dbg2 << "StandardReader::processPacket(): Running as leader!" << endl;

      if ( ctrlPacket ) {
         packetProcessed = leaderProcessCtrlPacket(p);
      } else {
         packetProcessed = leaderProcessNonCtrlPacket(p);
      }

   } else {
      // Available
      mc2dbg2 << "StandardReader::processPacket(): Running as available!" << endl;
      if (ctrlPacket) {
         packetProcessed = availableProcessCtrlPacket(p);
      } else {
         packetProcessed = availableProcessNonCtrlPacket(p);
      }
   }

   // Check if the packet have been processed or not
   if (!packetProcessed) {
      mc2log << warn << "StandardReader::processPacket():  Packet not processed."
             << " Packet type: " << endl;
      mc2log << p->getSubTypeAsString() << endl;
   }

   setLogPrefix(NULL);
}

void
StandardReader::replyToVote( const VotePacket* vp )
{
   VotePacket* reply = new VotePacket( m_ownAddr.getIP(),
                                       m_ownAddr.getPort(),
                                       m_definedRank,
                                       true ); // direct
   sendAndDeletePacket( reply, vp->getOriginAddr() );
}

void
StandardReader::processVotePacket(VotePacket* p,
                          bool wasLeader)
{

   mc2dbg << "processVotePacket" << endl;

   if ( p->getOriginAddr() == m_ownAddr ) {
      mc2dbg4 << "StandardReader::processVotePacket(): own vote received" << endl;
      return;
   }
   
   if ( wasLeader ) {
      mc2log << warn << "StandardReader::processVotePacket(): Leader: "
             << p->directOrMultiDbg() << " vote received from "
             << p->getOriginAddr() << endl;
   } else {
      mc2dbg8 << "StandardReader::processVotePacket(): Available: vote received"
              << " from " << p->getOriginAddr() << endl;
   }
   
   if ( p->betterThanThis(m_definedRank, m_ownAddr.getPort()) > 0) {
      // This module was better leader.
      mc2dbg4 << "StandardReader::processVotePacket(): I'm better leader!" << endl;
      
      // Only reply to the sender of the packet.
      replyToVote( p );
      
      // I have won at least one.
      m_internalStatus = WON_1;
      // Make voting quicker. Turn the clock forwards.
      m_lastHeartBeat -= MIN_WAIT_NBR_HEARTBEATS * HEARTBEAT_RATE;
   } else {
      mc2dbg << info << "[StandardReader]: Lost voting to a "
             << p->directOrMultiDbg() << " votepacket from "
             << p->getOriginAddr() << endl;
      
      if ( wasLeader ) {
         becomeAvailable();
      }
      m_internalStatus = LOST;
      // Has to be done as there are no Leader
      // at this time! To avoid voting, that is.
      m_lastHeartBeat = TimeUtility::getCurrentTime();
   }
}

bool 
StandardReader::availableProcessCtrlPacket(Packet* pa)
{
   auto_ptr<Packet> p(pa);
   switch((int) p->getSubType()) {
      case Packet::PACKETTYPE_HEARTBEAT : {
         
         m_loadedMaps->setLeaderAddr( pa->getOriginAddr() );

         mc2dbg4 << "StandardReader::availableProcessCtrlPacket():"
                 << " Heartbeat received" << endl;
         m_lastHeartBeat = TimeUtility::getCurrentTime();
         // Check that jobthread is still alive.
         if ( ! jobThreadWorkingTooLong() ) {
            replyToHeartBeat(); // This may be overridden by subclasses.
         } else {
            mc2dbg << "workthread working too long!" << endl;
         }
         return true;
      }
      break;

      case Packet::PACKETTYPE_VOTE : 
         processVotePacket((VotePacket*)p.get(), false);
         return true;
         break;

      case Packet::PACKETTYPE_IGNORE :
         mc2dbg4 << "StandardReader::availableProcessCtrlPacket(): Ignore received"
                 << endl;
         // Put the module into the "ignore-list"
         return true;
         break;
         
      case Packet::PACKETTYPE_STATISTICS : 
         mc2dbg4 << "StandardReader::availableProcessCtrlPacket(): "
            "Statistics received, "
                 << "this is wrong, does nothing" << endl;
         // This is wrong -- do nothing.
         return true;
         break;
      case Packet::PACKETTYPE_LEADERIP_REQUEST: {
         const LeaderIPRequestPacket& lrp =
            static_cast<const LeaderIPRequestPacket&>( *p.get() );

         ReplyPacket* reply =
            new LeaderIPReplyPacket( lrp,
                                     m_ownAddr.getIP(),
                                     m_ownAddr.getPort(),
                                     lrp.getModuleType(),
                                     m_loadedMaps->getLeaderAddr() );
         mc2dbg << "[StandardReader]: Avail - replying to LeaderIP sender "
                << reply->getOriginAddr() << " I am "
                << m_ownAddr << endl;
         sendAndDeletePacket( reply, reply->getOriginAddr() );
         return true;
      } break;
      default:
         // Will enqueue the packet in the queueueue.
         sendAndDeletePacket(p.release(), m_ownAddr);
         return true;
         break;
   }

   // How can we get here?
   MC2_ASSERT( false );
   return false;

}

bool 
StandardReader::availableProcessNonCtrlPacket(Packet* p)
{
   mc2dbg2 << "StandardReader::availableProcessingNonCtrlPacket   " 
          << p->getSubTypeAsString() << endl;

   sendAndDeletePacket(p, m_ownAddr);
   return (true);
}

inline AliveReplyPacket*
StandardReader::processAliveRequestPacket(const AliveRequestPacket* arp) const
{
   return new AliveReplyPacket(arp, m_moduleType);
}

bool 
StandardReader::leaderProcessCtrlPacket(Packet* p)
{
   switch ( (int) p->getSubType() ) {
      case Packet::PACKETTYPE_HEARTBEAT: {
         mc2dbg8 << "StandardReader::leaderProcessCtrlPacket(): Heartbeat received"
                 << endl;
         // This replyToHeartBeat is intended to reply to
         // it's own heartbeat for the Supervisor.
         replyToHeartBeat(); // This may be overridden by subclasses.
         HeartBeatPacket* hbp = (HeartBeatPacket*)p;
         if ( hbp->getOriginIP() == m_ownAddr.getIP() &&
              hbp->getOriginPort() == m_ownAddr.getPort() ) {
            // Ignore my own heartbeats, please.
            mc2dbg8 << "StandardReader::LeaderProcessCtrlPacket(): I have "
                    << m_balancer->getNbrModules() << " availables" << endl;
            m_loadedMaps->setLeaderAddr( m_ownAddr );
         } else {
            // Found other leader
            // Change this to avoid voting when leader that only
            // has itself is coming into the cluster.
            mc2dbg << "StandardReader::LeaderProcessCtrlPacket(): Other leader found"
                   << endl;
            mc2dbg << "StandardReader::LeaderProcessCtrlPacket(): I currently have "
                   << m_balancer->getNbrModules() << " availables" << endl;
            int rivalNbrModules = hbp->getNbrModules();
            mc2dbg << "StandardReader::LeaderProcessCtrlPacket(): The other one has "
                   << rivalNbrModules << endl;
            mc2dbg << "My rank " << m_definedRank
                   << " other " << hbp->getRank()
                   << endl;
            // Note that ZERO_AVAILABLES can have the value 1 if the
            // leader also is in the module list after Hpalm's changes
            bool iHaveZero    = m_balancer->getNbrModules() == ZERO_AVAILABLES;
            bool rivalHasZero = rivalNbrModules == ZERO_AVAILABLES;
            // Vote if both have zero or both have more than zero modules
            // and if we don't have the same rank
            if(m_definedRank > hbp->getRank()){
               // Ignore
            } else if ( (m_definedRank < hbp->getRank()) || 
                        ( ( m_definedRank == hbp->getRank() ) && 
                          ( iHaveZero   && !rivalHasZero  ) )) {
               // Resign - the other one has more modules and I have
               // probably been plugged in a short while ago.
               becomeAvailable();
            } else if (( m_definedRank == hbp->getRank() ) &&
                       (!iHaveZero     && rivalHasZero   ) ) {
               // Ignore the other one - the rival will resign since will
               // be in the case above.
            } else {
               // either not the same rank or neither of us have
               // zero modules, vote...
               // Don't become available until the vote is finished.
               // This is to avoid the long loading of index.db.
               //becomeAvailable();
#if 0
               // Send out my votepacket
               sendVoteToAll();
               // Make voting quicker. Turn the clock forwards.
               m_lastHeartBeat -= MIN_WAIT_NBR_HEARTBEATS * HEARTBEAT_RATE;
#else
               // Try a new trick
               VotePacket vp( hbp->getOriginIP(),
                              hbp->getOriginPort(),
                              hbp->getRank(), true );
               processVotePacket( &vp, true );
#endif
               
            } 
         }
         delete p;
         return (true);
      }
      break;
      case Packet::PACKETTYPE_VOTE: {
         processVotePacket( (VotePacket*)p, true);
         delete p;
         return (true);
      }
      break;
      case Packet::PACKETTYPE_IGNORE :
         mc2dbg4 << "StandardReader::leaderProcessCtrlPacket(): Ignore received" 
                 << endl;
         delete p;
         return (true);
      break;
      case Packet::PACKETTYPE_STATISTICS : {
         
         mc2dbg4 << "StandardReader::leaderProcessCtrlPacket(): Statistics received" 
                 << endl;
         PacketSendList packets;
         bool updateOK = m_balancer->updateStats( (StatisticsPacket*) p,
                                                  packets );
         sendAndDeletePackets( packets );
         return updateOK;
      }
      break;

      case Packet::PACKETTYPE_LOADMAPREPLY : {
         // Indicates that a map has been loaded by the sender.
         // Does nothing because the id will come in the 
         // statistics-packet from that module.

         // Maybe we should do something anyway. To try to avoid
         // "All modules are loading task on map X refused"

         mc2dbg4 << "StandardReader::leaderProcessCtrlPacket(): Loadmap received" 
                 << endl;
         reactToMapLoaded(static_cast<LoadMapReplyPacket*>(p));
         return (true);
      }
      break;
      case Packet::PACKETTYPE_DELETEMAPREPLY : {
         // Indicates that a map has been loaded by the sender.
         // Does nothing because the id will come in the 
         // statistics-packet from that module.

         mc2dbg4 << "StandardReader::leaderProcessCtrlPacket(): Deletemap received" 
                 << endl;
         delete p;
         return (true);
      }      
      break;
      case Packet::PACKETTYPE_LEADERIP_REQUEST : {
         LeaderIPRequestPacket* req = 
            static_cast<LeaderIPRequestPacket*>( p );
         if ( req->regardingCtrlPacket() &&
              req->getMapID() == MAX_UINT32 ) {
            // Send my own address since the leader must have
            // a look at the packet.
            ReplyPacket* reply = 
               new LeaderIPReplyPacket( *req,
                                        m_ownAddr.getIP(),
                                        m_ownAddr.getPort(),
                                        req->getModuleType(),
                                        m_loadedMaps->getLeaderAddr() );
            sendAndDeletePacket( reply, reply->getOriginAddr() );
         } else {
            sendRequestPacket( req );
         }
         return true;
      }
      break;
      case Packet::PACKETTYPE_ALIVEREQUEST: {
         AliveRequestPacket* arp = static_cast<AliveRequestPacket*>(p);
         AliveReplyPacket* reply = processAliveRequestPacket(arp);
         sendAndDeletePacket( reply, arp->getOriginAddr() );
         delete p;
         return true;
      }
      break;
   }

   // The packet type was not among the ones that are handled here
   mc2dbg << "StandardReader::leaderProcessingCtrlPacket():  " 
          << p->getSubTypeAsString() << endl;
   // Packet is not deleted.
   return (false);

}

bool 
StandardReader::leaderProcessNonCtrlPacket(Packet* p)
{
   mc2dbg4 << "StandardReader::leaderProcessingNonCtrlPacket(): " 
           << p->getSubTypeAsString() << endl;
   
   switch(p->getSubType()){
      case Packet::PACKETTYPE_TESTREQUEST :
         static_cast<TestRequestPacket*>(p)
            ->setTime(TimeUtility::getCurrentMicroTime());
         sendRequestPacket( (RequestPacket*)p );
         break;
      default:   
         sendRequestPacket( (RequestPacket*)p );
         break;
   }

   // The packet is always handled here!
   return (true);
}

void StandardReader::becomeLeader()
{
   bool wasLeader = isLeader();
   if ( isLeader() == false ) {
      mc2log << info << "StandardReader::becomeLeader():  "
             << getName() << "Module switching to Leader" 
             << endl;
   }
   m_leaderStatus->becomeLeader();
   m_packetReader.becomeLeader();
   m_internalStatus = NO_VOTING;
   m_balancer->becomeLeader();
   
   if ( !wasLeader && m_balancer->usesMaps() ) {
      set<MapID> allMaps;
      initLeader( allMaps );
      m_balancer->setAllMaps( allMaps );
   }
}

void StandardReader::becomeAvailable()
{
   if ( isLeader() == true ) {
      mc2log << info << "StandardReader::becomeAvailable(): "
             << getName() << "Module switching to "
                "Available" << endl;
   }
   m_leaderStatus->becomeAvailable();
   m_packetReader.becomeAvailable();
   m_balancer->becomeAvailable();
}

AllMapReplyPacket*
StandardReader::requestAllMapPacket()
{
   DatagramSender udpSender;
   DatagramReceiver udpReceiver( MultiCastProperties::changeMapSetPort( 8000 ), 
                                 DatagramReceiver::FINDFREEPORT );

   auto_ptr<AllMapReplyPacket> reply( new AllMapReplyPacket() );

   AllMapRequestPacket req( NetUtility::getLocalIP(),
                            udpReceiver.getPort() );
   uint32 mapip = MultiCastProperties::getNumericIP( MODULE_TYPE_MAP,
                                                     true );
   uint16 mapport = MultiCastProperties::getPort( MODULE_TYPE_MAP, true );
   uint32 mapSet = Properties::getMapSet();
   
   if (mapSet != MAX_UINT32) {
      // code also exists in PacketContainer.cpp and
      // ModuleMap.cpp, move to utility function?
      IPnPort before( mapip, mapport );
      IPnPort newaddr = MultiCastProperties::
         changeMapSetAddr( IPnPort( mapip, mapport ) );

      mapip = newaddr.getIP();
      mapport = newaddr.getPort();
      mc2dbg << "[StandardReader]: Changed map module addr from "
             << before << " -> " << newaddr
             << " because mapSet = " << mapSet;
   }
      
   const int maxNbrAttempts = 2;
   for ( int i = 1; i <= maxNbrAttempts; ++i ) {
      mc2dbg << "[StandardReader]: Sending AllMapRequestPacket "
             << i << " of " << maxNbrAttempts << endl;
      if ( ! udpSender.send( &req, mapip, mapport ) ) {
         mc2log << error << "[StandardReader]: Allmap sending to "
                << IPnPort( mapip, mapport ) << " failed" << endl;
         continue;
      }
      bool udpsuccess = udpReceiver.receive(reply.get(), 10000000);
      if ( ! udpsuccess ) {
         mc2log << error << "[StandardReader]: Allmap reception failed "
                << " make sure there is a map leader on "
                << IPnPort( mapip, mapport ) << endl;
         continue;
      }
      if ( reply->getSubType() != Packet::PACKETTYPE_ALLMAPREPLY ) {
         mc2log << error << "[StandardReader]: Allmap had type "
                << uint32(reply->getSubType()) << " instead of "
                << uint32(Packet::PACKETTYPE_ALLMAPREPLY) << "!" << endl;
         continue;
      }
      // OK! Return it
      if ( i != 0 ) {
         mc2dbg << "[StandardReader]: Received AllMapReplyPacket after "
                << i << " of " << maxNbrAttempts << " attempts" << endl;
      }

      if ( mapSet != MAX_UINT32 && reply->getNbrMaps() > 0 ) {
         uint32 replyMapSet = MapBits::getMapSetFromMapID( reply->getMapID( 0 ) );
         if ( replyMapSet != mapSet ) {
            mc2log << error << "[StandardReader]: AllMapReply had wrong mapSet(" 
                   << replyMapSet << "), but we want " << mapSet << endl;
            continue;
         }
      }

      return reply.release();
   }
   return NULL;
}

void StandardReader::initLeader(set<MapID>& allMaps)
{
   auto_ptr<AllMapReplyPacket> reply ( requestAllMapPacket() );

   allMaps.clear();

   if ( reply.get() == NULL ) {
      // Failed!
      becomeAvailable();
      return;
   }
   
   uint32 nbrMaps = reply->getNbrMaps();
   for (uint32 i=0; i<nbrMaps; i++) {
      uint32 mapID = reply->getMapID(i);
      mc2dbg << "[StandardReader::initLeader()] Adding mapID: "
             << prettyMapIDFill(mapID) << endl;

      if ( usesCountryMaps() || !MapBits::isCountryMap( mapID ) ) {
         allMaps.insert( mapID );
      }
   }
}

void StandardReader::replyToHeartBeat()
{
   auto_ptr<StatisticsPacket> tmpPack( createStatisticsPacket() );
   mc2dbg8 << "StandardReader(leader="<< isLeader() <<")::replyToHeartBeat(): StatisticsPacket sent to leader" 
          << endl;
   sendToLeaders( *tmpPack );
}

void
StandardReader::sendRequestPacket( RequestPacket* request )
{
   PacketSendList packets;
   m_balancer->getModulePackets( packets, request );
   sendAndDeletePackets( packets );
}


void
StandardReader::reactToMapLoaded(LoadMapReplyPacket* replyPacket)
{
   // Only the old load balancing needs this.
   SimpleBalancer* simpleBalancer = dynamic_cast<SimpleBalancer*>(m_balancer);
   if ( simpleBalancer != NULL ) {
      PacketSendList packets;
      simpleBalancer->reactToMapLoaded( packets, replyPacket );
      sendAndDeletePackets( packets );
   }
   delete replyPacket;
}


void
StandardReader::setModuleName()
{
   m_moduleTypeName = ModuleTypes::moduleTypeToString( m_moduleType );
}

void
StandardReader::setLogPrefix(Packet* p) 
{
   static const char origs[13][3] = 
           {"IN","SM","SS","HS","WS","NS","TE","MS","TS","MO","IS","XS","TR"};
   if (p == NULL)
      static_cast<LogBuffer*>(mc2log.rdbuf())->setPrefix(NULL);
   else {
      sprintf(m_logPrefix, "[%s-%08X-%04X] ",
              origs[p->getRequestOriginator() & 0xff], 
              p->getRequestTimestamp(), p->getRequestID());
      static_cast<LogBuffer*>(mc2log.rdbuf())->setPrefix(m_logPrefix);
   }
}

void
StandardReader::periodicMethod() {
   // Reader does it's periodic stuff in run.
}

StatisticsPacket* StandardReader::createStatisticsPacket() {
   MapSafeVector* loadedMaps = getLoadedMaps();
  
   // Get the current statistics value
   uint32 curStat = getQueueLength();

   // Create the statistics packet that will be returned
   return new StatisticsPacket( m_ownAddr,
                                *loadedMaps,
                                m_optMem,
                                m_maxMem,
                                curStat,
                                getRank() );
}
