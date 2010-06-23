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

#include "ReaderBase.h"
#include "PacketSender.h"
#include "Packet.h"
#include "DatagramSocket.h"
#include "Queue.h"
#include "ModulePacketSender.h"

#include "DeleteMapPacket.h"
#include "LoadMapPacket.h"

#include "MapSafeVector.h"

ReaderBase::ReaderBase( Queue* jobQueue,
                        NetPacketQueue& sendQueue,
                        MapSafeVector* loadedMaps,
                        const IPnPort& ownAddr,
                        const IPnPort& leaderAddr,
                        const IPnPort& availableAddr,
                        bool pushWanted)
      : m_loadedMaps(loadedMaps), 
        m_ownAddr(ownAddr),
        m_packetQueue( jobQueue ),
        m_leaderAddr(leaderAddr),
        m_availAddr(availableAddr),
        m_pushWanted(pushWanted),
        m_packetSender( new ModulePacketSender( sendQueue, 
                                                Packet::getTCPLimitSize() ) )
{

}

ReaderBase::~ReaderBase()
{

}

void
ReaderBase::enqueue(Packet* p)
{
   bool checkResource = true;
   bool enqueuePacket = true;
   switch ( p->getSubType() ) {
      case Packet::PACKETTYPE_LOADMAPREQUEST: {
         LoadMapRequestPacket* lmrp =
            static_cast<LoadMapRequestPacket*>(p);
         m_loadedMaps->setStatus(lmrp->getMapID(),
                                 MapElement::TOLD_TO_LOAD);
         // Update last use
         m_loadedMaps->updateLastUse( lmrp->getMapID() );
         if( m_pushWanted ) {
            m_waitingForPush.insert(make_pair(lmrp->getMapID(), 
                                              vector<Packet*>()));
            uint32 pushTime = TimeUtility::getRealTime();
            uint32 mapID = lmrp->getMapID();
            m_pushTime.insert( make_pair( mapID, pushTime ) );
         }
         checkResource = false;
      }
      break;
      case Packet::PACKETTYPE_DELETEMAPREQUEST: {
         DeleteMapRequestPacket* dmrp =
            static_cast<DeleteMapRequestPacket*>(p);
         m_loadedMaps->setStatus(dmrp->getMapID(),
                                 MapElement::TOLD_TO_DELETE);
         if( m_pushWanted ) {
            m_waitingForPush.erase(dmrp->getMapID());
            m_pushTime.erase(dmrp->getMapID());
         }
         checkResource = false;            
      }
      break;
      case Packet::PACKETTYPE_DISTURBANCEPUSH: {
         m_packetQueue->enqueue(p);
         uint32 mapID = static_cast<RequestPacket*>(p)->getMapID();
         map<uint32, vector<Packet*> >::iterator it;
         it = m_waitingForPush.find(mapID);
         if( it != m_waitingForPush.end() ) {
            vector<Packet*> packetVector = it->second;
            vector<Packet*>::iterator packetIt;
            for(packetIt = packetVector.begin();
                packetIt != packetVector.end(); packetIt++)
            {
               Packet* currentPacket = *packetIt;
               m_packetQueue->enqueue(currentPacket);
            }
         } 
         m_waitingForPush.erase(mapID);
         m_pushTime.erase(mapID);
         enqueuePacket = false;
      }
      break;
      default:
      break;
   }
   // Check if timeout
   uint32 timeNow = TimeUtility::getRealTime();
   set<uint32> removedMaps;
   map<uint32, uint32>::iterator timeIt;
   for(timeIt = m_pushTime.begin(); timeIt != m_pushTime.end(); timeIt++) {
      uint32 mapID = timeIt->first;
      uint32 currentTime = timeIt->second;
      if( (timeNow - currentTime) > 15 ) {
         map<uint32, vector<Packet*> >::iterator it;
         it = m_waitingForPush.find(mapID);
         if( it != m_waitingForPush.end() ) {
            vector<Packet*> packetVector = it->second;
            vector<Packet*>::iterator packetIt;
            for(packetIt = packetVector.begin();
                packetIt != packetVector.end(); packetIt++)
            {
               Packet* currentPacket = *packetIt;
               m_packetQueue->enqueue(currentPacket);
            }
         } 
         m_waitingForPush.erase(mapID);
         removedMaps.insert(mapID);
      }
   }
   set<uint32>::iterator mapIt;
   for(mapIt = removedMaps.begin(); mapIt != removedMaps.end(); mapIt++) {
      uint32 currentMapID = *mapIt;
      m_pushTime.erase(currentMapID);
   }
   
   if( enqueuePacket ) {
      if( m_pushWanted && checkResource ) {
         map<uint32, vector<Packet*> >::iterator it;
         RequestPacket* reqPacket = static_cast<RequestPacket*>(p);
         uint32 mapID = reqPacket->getMapID();
         it = m_waitingForPush.find(mapID);
         if( it != m_waitingForPush.end() ) {
            vector<Packet*> packetVector = it->second;
            packetVector.push_back(p);
            m_waitingForPush.erase(mapID);
            m_waitingForPush.insert( make_pair( mapID, packetVector ) );
         } else {
            m_packetQueue->enqueue(p);
         }
      } else {
         m_packetQueue->enqueue(p);
      }
   }
}

void
ReaderBase::sendAndDeletePacket( Packet* p,
                                 const IPnPort& dest )
{
   if ( p == NULL ) {
      return;
   }

   if ( dest != m_ownAddr ) {
      m_packetSender->sendPacket( p, dest );
   } else {
      // The packet is for my own processor
      enqueue( p );      
   }
}

void
ReaderBase::sendAndDeletePackets( PacketSendList& packets ) {
   PacketSendList::iterator it = packets.begin();
   PacketSendList::iterator itEnd = packets.end();
   for (; it != itEnd; ++it ) {
      sendAndDeletePacket( it->second, it->first );
   }
}

void
ReaderBase::sendToAvailables( const Packet& p )
{
   m_packetSender->sendPacket( p.getClone(), m_availAddr );
}

void
ReaderBase::sendToLeaders( const Packet& p )
{
   m_packetSender->sendPacket( p.getClone(), m_leaderAddr );
}

int
ReaderBase::getQueueLength()
{
   return m_packetQueue->getStatistics();
}
