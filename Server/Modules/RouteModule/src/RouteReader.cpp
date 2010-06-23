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

#include "RouteReader.h"
#include "DebugRM.h"
#include "OrigDestNodes.h"
#include "Connection.h"
#include "ItemTypes.h"
#include "RouteConstants.h"
#include "DisturbanceSubscriptionPacket.h"
#include "Packet.h"
#include "AllMapPacket.h"
#include "DatagramSocket.h"
#include "Queue.h"
#include "Fifo.h"
#include "LoadMapPacket.h"
#include "multicast.h"

#include "MapNotice.h"
#include "MapBits.h"

RouteReader::RouteReader(Queue* q,
                         NetPacketQueue& sendQueue,
                         MapSafeVector* loadedMaps,
                         PacketReaderThread* packetReader,
                         uint16 listenPort,
                         uint32 definedRank,
                         char* logName,
                         bool superLeader,
                         vector<PushService*>* wantedServices,
                         bool waitForPush):
      StandardReader( MODULE_TYPE_ROUTE,
                      q, sendQueue,
                      loadedMaps,
                      *packetReader,
                      listenPort,
                      definedRank,
                      true, // Uses maps
                      wantedServices,
                      waitForPush)
{
   if (logName != NULL) {
      m_logName = new char[strlen(logName)+1];
      strcpy(m_logName, logName);
   }
   else {
      m_logName = NULL;
   }
   m_superLeader = superLeader;
   if ( superLeader == false ) {
      mc2dbg << "Will send SubRoutePackets to availables if I get any"
             << endl;
   }
}


RouteReader::~RouteReader()
{
   delete [] m_logName;
}


bool
RouteReader::leaderProcessCtrlPacket(Packet* p)
{
   mc2dbg4 << "RouteReader::leaderProcessCtrlPacket" << endl;
   bool handled = StandardReader::leaderProcessCtrlPacket(p);
   if (!handled) {      
      switch (p->getSubType()) {
         case Packet::PACKETTYPE_SUBROUTEREPLY : {
            mc2dbg << "Leader got SubRouteReplyPacket" << endl;
            handled = true;
         }
         break;

         default : {
            mc2log << warn << "Leader does not recognize packettype"
                   << endl << "subType " << (int)p->getSubType()
                   << endl;
         }
         break;
      } // end switch
      
      if (handled) {
         delete p;
      }
   } // end if (!handled)
   return handled;
} // leaderProcessCtrlPacket


bool
RouteReader::leaderProcessNonCtrlPacket(Packet* packet)
{
   switch (packet->getSubType()) {
      
      case Packet::PACKETTYPE_TESTREQUEST : {
         mc2dbg1 <<"Leader got TestRequestPacket" << endl;
         delete packet;
         packet = NULL;
      }
      break;
      
      case Packet::PACKETTYPE_DISTURBANCESUBSCRIPTIONREPLY : {
         mc2dbg2 << "Leader got DisturbanceSubscriptionReplyPacket" << endl;
         delete packet;
         packet = NULL;
      }
      break;
      
      case Packet::PACKETTYPE_SUBROUTEREQUEST : {
         // The packet will be deleted in sendRequestPacket
         sendRequestPacket(static_cast<RequestPacket*>(packet));

      }
      break;
      
      default : {
         sendRequestPacket(static_cast<RequestPacket*>(packet));
      }
      break;
   }   
   return true;
} // leaderProcessNonCtrlPacket


void
RouteReader::printOrigDests(Head* originList,
                            Head* destList)
{
   mc2log << "origins : " << endl;
   if ( originList != NULL ) {
      for(OrigDestNode* cur = static_cast<OrigDestNode*>(originList->first());
          cur != NULL;
          cur = static_cast<OrigDestNode*>(cur->suc()) ) {
         ((OrigDestNode*)cur)->dump();
      }
   }
   mc2log << "destinations : " << endl;
   if ( destList != NULL ) {
      for(OrigDestNode* cur = static_cast<OrigDestNode*>(destList->first());
          cur != NULL; 
          cur = static_cast<OrigDestNode*>(cur->suc())) {
         ((OrigDestNode*)cur)->dump();
      }
   }
}

