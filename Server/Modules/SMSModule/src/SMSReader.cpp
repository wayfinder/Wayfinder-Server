/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Packet.h"
#include "SMSPacket.h"
#include "DatagramSocket.h"
#include "StatisticsPacket.h"
#include "PacketQueue.h"

#include "SMSReader.h"

#include "SMSSimpleBalancer.h"


SMSReader::SMSReader(Queue *q, NetPacketQueue& sendQueue,                     
                     MapSafeVector* loadedMaps,
                     PacketReaderThread* packetReader,
                     uint16 port,
                     uint32 definedRank,
                     int nbrPhones,
                     int nbrServices,
                     char** phones,
                     char** services)
   
      :  StandardReader(  MODULE_TYPE_SMS,
                          q, sendQueue,                  
                          loadedMaps,
                          *packetReader,
                          port,
                          definedRank,
                          false)
{
   // Change balancer to the SMSBalancer.

   
   mc2dbg2 << "Creating SMSReader" << endl;
   numberOfPhones = nbrPhones;
   numberOfServices = nbrServices;

   // Allocate and copy the phonenumbers and services.
   phoneNumbers = new char*[nbrPhones];
   for ( int i=0; i < nbrPhones; i++ ) {
      phoneNumbers[i] = new char[strlen(phones[i]) + 1];
      strcpy(phoneNumbers[i], phones[i]);
      DEBUG2(cout << "  Phonenumber[" << i << "] = " << phoneNumbers[i] 
                  << endl);
   }
   serviceNames = new char*[nbrServices];
   for ( int i = 0; i < nbrServices; i++ ) {
      serviceNames[i] = new char[strlen(services[i]) + 1];
      strcpy( serviceNames[i], services[i] );
      DEBUG2(cout << "  ServiceName[" << i << "] = " << serviceNames[i] 
                  << endl);      
   }

   delete m_balancer;
   std::auto_ptr<SMSStatisticsPacket> statPacket( 
      static_cast<SMSStatisticsPacket*>( createStatisticsPacket() ) );
   m_balancer = new SMSSimpleBalancer(m_ownAddr,
                                      getName(),
                                      statPacket.get(),
                                      numberOfPhones,
                                      numberOfServices,
                                      phoneNumbers,
                                      serviceNames);
                                    
   mc2dbg2 << "SMSReader created" << endl;
}


SMSReader::~SMSReader()
{
   for( int i = 0; i < numberOfServices; i++ ) {
      delete serviceNames[i];
      serviceNames[i] = NULL;
   }
   for ( int i = 0; i < numberOfPhones; i++ ) {
      delete phoneNumbers[i];
      phoneNumbers[i] = NULL;
   }
   delete phoneNumbers;
   phoneNumbers = NULL;
   delete serviceNames;
   serviceNames = NULL;
}

bool 
SMSReader::leaderProcessCtrlPacket(Packet*p)
{
   // Check the indata
   if (p == NULL) {
      mc2log << error << "SMSReader::leaderProcessCtrlPacket, Packet == NULL" 
             << endl;
      return false;
   }

   if ( p->getSubType() == Packet::PACKETTYPE_SMSSTATISTICS ) {
      // It is a SMSStatisticsPacket
      PacketSendList packets;
      bool updateOK = m_balancer->updateStats( (StatisticsPacket*) p,
                                               packets );
      
      for( PacketSendList::iterator it = packets.begin();
           it != packets.end();
           ++it ) {
         sendAndDeletePacket(it->second, it->first);
      }
      
      return updateOK;     
   } else {  
      mc2dbg8 << "SMSReader::leaderProcessCtrlPacket - type ="
              << p->getSubTypeAsString() << " Sender = "
              << p->getOriginIP() << ","
              << p->getOriginPort() << endl;
      mc2dbg8 << "SMSReader - calling Reader::leaderProcessCtrlPacket()"
              << endl;

      if ( StandardReader::leaderProcessCtrlPacket(p) == false ) {
         // The packet wasn't taken care of
         DEBUG1(cerr << "SMSReader - no one took care of the packet"
                     << endl);
         delete p;
         p = NULL;
         return (false);
      } else {
         // The packet was handled by the superclass (Reader)
         // And deleted?
         return (true);
      }
   }
}


void
SMSReader::replyToHeartBeat()
{
   auto_ptr<StatisticsPacket> tmpPack( createStatisticsPacket() );
   mc2dbg8
      << "SMSReader::replyToHeartBeat(): SMSStatisticsPacket sent to leader" 
      << endl;
   sendToLeaders(*tmpPack);
}

void SMSReader::printPacket(Packet* p)
{
  bool isSendRequest =
    p->getSubType() == Packet::PACKETTYPE_SMSREQUEST;
  bool isListenRequest =
    p->getSubType() == Packet::PACKETTYPE_SMSLISTENREQUEST;
  if ( isSendRequest ) {
     SMSSendRequestPacket *srp = (SMSSendRequestPacket*)p;
     // Decode the packet
     int encodingType = CONVERSION_NO;
     char* senderPhone = NULL;
     char* recipientPhone = NULL;
     int dataLength = 0;
     byte* data;
     srp->getPacketContents(encodingType, senderPhone,
                            recipientPhone, dataLength,
                            data);
     delete [] data;
     mc2log << info 
            << "  SendRequestPacket: senderPhone :" << senderPhone << endl
            << "  recipientPhone : " << recipientPhone << endl;
  } else if ( isListenRequest ) {
    SMSListenRequestPacket *slrp = (SMSListenRequestPacket*)p;
    char* phoneNumber = NULL;
    slrp->getPhoneNumber(phoneNumber);
    mc2log << info << "  ListenRequestPacket: phoneNumber :"  << phoneNumber
           << endl;
  }
}

StatisticsPacket*
SMSReader::createStatisticsPacket() {
   // Get the current statistics value
   uint32 curStat = getQueueLength();
   
   // Create the statistics packet that will be returned
   return new SMSStatisticsPacket( m_ownAddr,
                                   *getLoadedMaps(),
                                   curStat,
                                   numberOfPhones,
                                   phoneNumbers,
                                   numberOfServices,
                                   serviceNames );
}
