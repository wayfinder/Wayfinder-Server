/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SMSSendRequest.h"

const uint32 SMSSendRequest::SMSSENDREQUESTPACKET_TIMEOUT = 30000;

SMSSendRequest::SMSSendRequest( uint16 requestID)
      : Request( requestID )
{
   m_answer = NULL;
   m_firstSendPacket = NULL;
   curSMSPacketContainer = NULL;
   m_nbrSendPackets= 0;
   m_nbrReceivedPackets= 0;
}
   
SMSSendRequest::SMSSendRequest( uint16 requestID,
                                SMSSendRequestPacket* p ) 
      : Request( requestID )
{
   m_answer = NULL;
   m_firstSendPacket = NULL;
   curSMSPacketContainer = NULL;
   m_nbrSendPackets= 0;
   m_nbrReceivedPackets= 0;
   
   addNewSMS(p);
}


SMSSendRequest::~SMSSendRequest()
{
   while ( curSMSPacketContainer != NULL ) {
      SMSPacketContainer* next = curSMSPacketContainer->getNext();
      delete curSMSPacketContainer;
      curSMSPacketContainer = next;
   }
}

bool
SMSSendRequest::addNewSMS(SMSSendRequestPacket* pack)
{
   if (pack != NULL) {
      // Create the SMSPacketContainer for this packet
      uint16 tmpPackID = getNextPacketID();      
      pack->setRequestID( getID() );
      pack->setPacketID( tmpPackID );
      
      SMSPacketContainer* newCont = 
            new SMSPacketContainer( pack,
                                    0,
                                    0,
                                    MODULE_TYPE_SMS,
                                    SMSSENDREQUESTPACKET_TIMEOUT); 

      // Insert the packetContainer into the list of SMSPacketContainers
      if (m_firstSendPacket == NULL) {
         m_firstSendPacket = newCont;
         curSMSPacketContainer = m_firstSendPacket;
         m_nbrSendPackets = 1;
      } else {
         SMSPacketContainer* cur = m_firstSendPacket;
         while (cur->getNext() != NULL) {
            cur = cur->getNext();
         }
         cur->setNext(newCont);
         m_nbrSendPackets++;
      }
      return (true);
   } else {
      return (false);
   }
}


PacketContainer*
SMSSendRequest::getNextPacket() 
{
   SMSPacketContainer* retVal = curSMSPacketContainer;
   if (curSMSPacketContainer != NULL)
      curSMSPacketContainer = curSMSPacketContainer->getNext();

   DEBUG1(
      SMSSendRequestPacket* p;
      if ( (retVal != NULL) && 
           ((p = dynamic_cast<SMSSendRequestPacket*>
                 (retVal->getPacket())) != NULL)){
         const int maxLen = 1023;
         char* s = new char[maxLen+1];
         DEBUG2( cerr << p->printSMS(s, maxLen) << endl );
         delete [] s;
      }
   );

   return (retVal);
}


void
SMSSendRequest::processPacket(PacketContainer *ans) {
   if ( ans != NULL ) {
      if (ans->getPacket()->getSubType() == Packet::PACKETTYPE_SMSREPLY) {
         m_nbrReceivedPackets++;
         m_done = (m_nbrReceivedPackets == m_nbrSendPackets);
      }

      // If done then update the PacketContainer that is returned from 
      // this request.
      if (m_done) {
         m_answer = (SMSPacketContainer*) ans;
      } else {
         delete ans;
      }
   }
}


PacketContainer*
SMSSendRequest::getAnswer() {
   return (m_answer);
}


uint32 
SMSSendRequest::getNbrSMSs() {
   return m_nbrSendPackets;
}


bool 
SMSSendRequest::requestDone() {
   if ( m_nbrSendPackets == 0 ) {
      m_done = true;
   }
   return m_done;
}


SMSSendRequestPacket* 
SMSSendRequest::getSMSSendRequestPacket( uint32 index ) {
   SMSPacketContainer* retVal = m_firstSendPacket;
   uint32 pos = 0;

   while ( retVal != NULL && pos < index ) {
      retVal = retVal->getNext();
      pos++;
   }

   if ( retVal != NULL ) {
      return static_cast< SMSSendRequestPacket* > ( retVal->getPacket() );
   } else {
      return NULL;
   }
}
