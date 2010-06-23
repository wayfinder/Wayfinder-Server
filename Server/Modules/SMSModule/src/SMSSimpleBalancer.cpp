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

#include "SMSSimpleBalancer.h"
#include "StatisticsPacket.h"
#include "SMSPacket.h"
#include "ModuleList.h"
#include "SMSModuleNotice.h"
#include "SMSModuleList.h"
#include "StringTable.h"
#include "DeleteHelpers.h"

// --

SMSSimpleBalancer::SMSSimpleBalancer(const IPnPort& ownPort,
                                     const MC2String& moduleName,
                                     SMSStatisticsPacket* ownStatistics,
                                     int nbrPhones,
                                     int nbrServices,
                                     char** phones,
                                     char** services)
   : SimpleBalancer(ownPort, moduleName, ownStatistics, false),
     m_nbrPhones( nbrPhones ),
     m_nbrServices( nbrServices ),
     m_phones( phones ),
     m_services( services )
{
   createSMSModuleList( ownStatistics );
}

bool
SMSSimpleBalancer::updateStats(StatisticsPacket* packet, PacketSendList& )
{
   mc2dbg8 << "   SMSReader::leaderProcessCtrlPacket: "
           << "SMSSTATISTICS recv" << endl;
   SMSStatisticsPacket* sp = (SMSStatisticsPacket*)packet;
   SMSModuleNotice* mn = dynamic_cast<SMSModuleNotice*>
      (m_moduleList->findModule( sp->getOriginIP(), sp->getOriginPort()));
   
   if ( mn == NULL ) {
      mc2dbg << "   SMSReader::leaderProcessCtrlPacket: new notice"
             << endl;
      mc2dbg << "                 IP   = " << sp->getOriginIP()
             << "                 PORT = " << sp->getOriginPort()
             << endl;
      mn = new SMSModuleNotice(sp);
      m_moduleList->push_back( mn );
   } else {
      DEBUG8(cout << "   SMSReader::leaderProcessCtrlPacket: updating"
             << " notice" << endl);  
      mn->updateData(sp);
   }
   delete packet;
   return true;
}

bool
SMSSimpleBalancer::getModulePackets(PacketSendList& packets,
                                    RequestPacket* req)
{
   bool isSendRequest =
      req->getSubType() == Packet::PACKETTYPE_SMSREQUEST;
   bool isListenRequest =
      req->getSubType() == Packet::PACKETTYPE_SMSLISTENREQUEST;

   if ( !isListenRequest && !isSendRequest ) {
      return false;
   }
   
   // This function should choose the best module for the job.
   SMSModuleNotice *mn = (SMSModuleNotice *)
      (m_moduleList->getBestModule(req));
   bool moduleFound = false;
   
   if (mn != NULL) {
      moduleFound = true;
      mn->addPacketToQueue( MAX_UINT32 );

      IPnPort dest(mn->getIP(), mn->getPort());
      mc2dbg << "[SMSSimpleBalancer]: Dist job to "
             << dest << endl;
      packets.push_back( make_pair(dest, req) );
      req = NULL;
   } else {
      // Haven't found it.
      mc2dbg1 << "  SMSReader : Couldn't find a suitable module"
              << endl;                
      // Send errorpacket
      if ( isListenRequest ) {
         // SMSListenRequestPacket
         DEBUG1(cerr << "  Sending SMSListenReplyPacket with status"
                << " StringTable::NOTFOUND" << endl);
         SMSListenReplyPacket* slrp = (SMSListenReplyPacket*)req;
         slrp->setSubType(Packet::PACKETTYPE_SMSLISTENREPLY);
         slrp->setStatus(StringTable::NOTFOUND);
         IPnPort dest(slrp->getOriginIP(), slrp->getOriginPort());
         packets.push_back( make_pair(dest, slrp) );
      } else {
         // SMSSendRequestPacket
         mc2dbg1 << "  Sending SMSSendReplyPacket with status"
                 << " StringTable::NOTFOUND" << endl;
         SMSSendReplyPacket* srp = (SMSSendReplyPacket*)req;
         srp->setSubType(Packet::PACKETTYPE_SMSREPLY);
         srp->setStatus(StringTable::NOTFOUND);
         IPnPort dest(srp->getOriginIP(), srp->getOriginPort());
         packets.push_back( make_pair(dest, srp) );
      }
   }
   return true;
}

void
SMSSimpleBalancer::createSMSModuleList( SMSStatisticsPacket* ownStatistics )
{
   STLUtility::deleteValues( *m_moduleList );
   m_moduleList.reset( new SMSModuleList() );
   
   // Create and add a new leader notice to the list.
   SMSModuleNotice* leaderNotice = new SMSModuleNotice( ownStatistics );
   m_moduleList->push_back( leaderNotice );
}
