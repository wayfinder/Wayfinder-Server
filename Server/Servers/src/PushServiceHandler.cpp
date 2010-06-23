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

#include "PushServiceHandler.h"
#include "SocketReceiver.h"
#include "DatagramSocket.h"
#include "PacketContainer.h"
#include "PacketReceiver.h"
#include "PushPacket.h"
#include "TCPSocket.h"
#include "NetUtility.h"


PushServiceHandler::PushServiceHandler( uint16 port ) {
   // Create the sockets
   m_datagramSender = new DatagramSender();

   m_ip = NetUtility::getLocalIP();
   m_listenSocket = new TCPSocket();
   uint16 res = 0;

   if ( !m_listenSocket->open() ) {
      mc2log << fatal << "PushServiceHandler::PushServiceHandler "
         "failed to create listenSocket." << endl;
      exit( 1 );
   }

   res = m_listenSocket->listen( port, TCPSocket::FINDFREEPORT );

   if ( res == 0 ) {
      mc2log << fatal << "PushServiceHandler::PushServiceHandler "
         "failed to find a port to listen on." << endl;
      exit( 1 );
   }
   m_listenPort = res;
   mc2log << info << "PushServiceHandler::PushServiceHandler push port "
          << m_listenPort << endl;

   m_PushServices = new PushServices();
   m_pushServiceThread = new PushServiceThread( this, m_listenSocket );
}


PushServiceHandler::~PushServiceHandler() {
   ISABThreadHandle threadHandle = m_pushServiceThread;
   
   m_pushServiceThread->terminateSocketSelection();
   m_pushServiceThread->terminate();

   // Wait for m_pushServiceThread to end
   while ( threadHandle->isAlive() ) {
      ISABThread::yield();
   }

   delete m_PushServices;
   delete m_listenSocket;
   delete m_datagramSender;
}


void 
PushServiceHandler::start() {
   m_pushServiceThread->start();  
}


PushPacket*
PushServiceHandler::getPushPacket( uint32& serviceID, 
                                   SubscriptionResource*& resource )
{
   ISABSync sync( m_getMonitor );
   resource = NULL;

   if ( m_pushPackets.empty() ) {
      try {
         m_getMonitor.wait();
      }
      catch(const JTCInterruptedException &) {
      }
   }
   
   if ( !m_pushPackets.empty() ) {
      PushPacketList::reference ref = m_pushPackets.front();
      serviceID = ref.first.first;
      resource = ref.first.second;
      PushPacket* res = ref.second;
      
      m_pushPackets.pop_front();

      return res;
   } else {
      return NULL;
   }
}


void
PushServiceHandler::notifyGetPushPacket() {
   ISABSync sync( m_getMonitor );
   m_getMonitor.notifyAll();
}


void
PushServiceHandler::addPushService( PushService* service,
                                    vector<uint32>& lastUpdateTime )
{
   PacketContainerList packetList;

   m_mutex.lock();
   m_PushServices->addPushService( service, packetList, lastUpdateTime );
   m_pushServiceThread->forceRestart();
   m_mutex.unlock();

   if ( !packetList.empty() ) {
      sendPackets( packetList );
   }
}


bool
PushServiceHandler::removePushService( uint32 serviceID ) {
   PacketContainerList packetList;
   bool exist = false;

   m_mutex.lock();
   exist = m_PushServices->removePushService( serviceID, packetList );
   m_pushServiceThread->forceRestart();
   m_mutex.unlock();

   if ( !packetList.empty() ) {
      sendPackets( packetList );
   }

   return exist;
}


bool
PushServiceHandler::removePushServiceResource( 
   uint32 serviceID, SubscriptionResource& resource )
{
   PacketContainerList packetList;
   bool exist = false;

   m_mutex.lock();
   exist = m_PushServices->removePushServiceResource( serviceID,
                                                      resource,
                                                      packetList );
   m_pushServiceThread->forceRestart();
   m_mutex.unlock();

   if ( !packetList.empty() ) {
      sendPackets( packetList );
   }

   return exist;  
}


void
PushServiceHandler::handlePushPacket( PushPacket* pushPacket, 
                                      TCPSocket* pushSocket,
                                      bool& isDataPacket,
                                      uint32& serviceID, 
                                      SubscriptionResource*& resource )
{
   PacketContainerList packetList;

   m_mutex.lock();
   m_PushServices->handlePushPacket( pushPacket, pushSocket, packetList,
                                     isDataPacket, serviceID, resource );
   // No need to restart pushServiceThread bq. it called this method.
   m_mutex.unlock();
   
   if ( !packetList.empty() ) {
      sendPackets( packetList );
   }
}


void
PushServiceHandler::handleBrokenSocket( TCPSocket* pushSocket ) {
   PacketContainerList packetList;
  
   m_mutex.lock();
   m_PushServices->handleBrokenSocket( pushSocket, packetList );
   // No need to restart pushServiceThread bq. it called this method.
   m_mutex.unlock();

   if ( !packetList.empty() ) {
      sendPackets( packetList );
   }
}


uint32
PushServiceHandler::checkAndCalculateTimeout() {
   PacketContainerList packetList;
   uint32 timeOut = MAX_UINT32;

   m_mutex.lock();
   timeOut = m_PushServices->checkAndCalculateTimeout( packetList );
   // No need to restart pushServiceThread bq. it called this method.
   m_mutex.unlock();

   if ( !packetList.empty() ) {
      sendPackets( packetList );
   }
  
   return timeOut;
}


void
PushServiceHandler::sendPackets( PacketContainerList& packetList ) {
   m_sendMutex.lock();
  
   while ( !packetList.empty() ) {
      PacketContainerList::reference ref = packetList.front();
      uint32 IP = 0;
      uint16 port = 0;
      ref->getIPAndPort( IP, port );
      ref->getPacket()->setOriginIP( m_ip );
      ref->getPacket()->setOriginPort( m_listenPort );
      m_datagramSender->send( ref->getPacket(), IP, port );

      delete ref;
      packetList.pop_front();
   }
   
   m_sendMutex.unlock();
}


void
PushServiceHandler::addPushPacket( PushPacket* pushPacket, 
                                   uint32 serviceID, 
                                   SubscriptionResource* resource )
{
   m_mutex.lock();
   m_pushPackets.push_back( make_pair( 
      make_pair( serviceID, resource ), pushPacket ) );
   ISABSync sync( m_getMonitor );
   m_getMonitor.notifyAll();
   m_mutex.unlock();
}


////////////////////////////////////////////////////////////////////////
// PushServiceThread
////////////////////////////////////////////////////////////////////////


PushServiceHandler::PushServiceThread::PushServiceThread( 
   PushServiceHandler* handler,
   TCPSocket* listenSocket )
      : m_handler( handler ),
        m_listenSocket( listenSocket )
{
   m_receiver = new SocketReceiver();
   m_receiver->addTCPSocket( m_listenSocket ); 
}


PushServiceHandler::PushServiceThread::~PushServiceThread() {
   delete m_receiver;

   while ( !m_addedSockets.empty() ) {
      delete m_addedSockets.front();
      m_addedSockets.pop_front();
   }
}


void
PushServiceHandler::PushServiceThread::run() {
   uint32 timeOutTime = MAX_UINT32;
   TCPSocket* tcpSock = NULL;
   DatagramReceiver* datagramSock = NULL;

   while ( !terminated ) {
      uint32 now = TimeUtility::getRealTime();
      timeOutTime = m_handler->checkAndCalculateTimeout();
      uint32 sleepTime = timeOutTime - now;
      uint32 microSleap = (sleepTime*1000000) < sleepTime 
         ? MAX_UINT32 : sleepTime*1000000;

      if ( m_receiver->select( microSleap, tcpSock, datagramSock )  ) {
         // Have socket
         if ( tcpSock != NULL ) {
            if ( tcpSock != m_listenSocket ) {
               // Read packet
               PushPacket* packet = NULL;
               bool isDataPacket = false;
               uint32 serviceID = 0;
               SubscriptionResource* resource = NULL;
               
               packet = static_cast<PushPacket*> ( 
                  PacketReceiver::readAndCreatePacket( tcpSock ) );

               if ( packet != NULL ) {
                  m_handler->handlePushPacket( packet, tcpSock, 
                                               isDataPacket,
                                               serviceID, resource );
                  if ( isDataPacket ) {
                     m_handler->addPushPacket( packet, 
                                               serviceID, resource );
                  } else { // No data, heartbeat.., don't send any further
                     delete packet;
                     delete resource;
                  }
               } else {
                  // Failed to read packet, close socket and resubscribe
                  tcpSock->close();
                  // Remove socket
                  m_addedSockets.remove( tcpSock );
                  m_receiver->removeTCPSocket( tcpSock );
                  m_handler->handleBrokenSocket( tcpSock );
                  delete tcpSock;
               }

               
            } else {
               // Accept new socket
               TCPSocket* tmpSock = m_listenSocket->accept( 5000000 );
               if ( tmpSock != NULL ) {
                  m_addedSockets.push_back( tmpSock );
                  m_receiver->addTCPSocket( tmpSock );
               }
            }
         }
      } // Timeout

   } // while !terminated
   
}


void 
PushServiceHandler::PushServiceThread::forceRestart() {
   m_receiver->forceTimeout();
}


void 
PushServiceHandler::PushServiceThread::terminateSocketSelection() {
   m_receiver->terminate();
}
