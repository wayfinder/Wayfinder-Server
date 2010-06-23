/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SMSProcessor.h"
#include "StringTable.h"
#include "MapSafeVector.h"

SMSProcessor::SMSProcessor(SMSTalker* smsTalker, MapSafeVector* loadedMaps)
      : Processor( loadedMaps ),  m_smsTalker(smsTalker)
{
   m_forwardThread = new SMSForwardThread;
   m_forwardThread->start();
}

SMSProcessor::~SMSProcessor()
{

}

// This function is not a member of the class SMSProcessor.
// It's a kind of souped up macro
static inline void
makePacketInfo(char* packetInfo,
               SMSSendRequestPacket* p)
{
   const char* senderPhone = NULL;
   const char* recipientPhone = NULL;
   int dataLength;
   byte* data;
   char* inSender = (char*)senderPhone;
   char* inRec = (char*)recipientPhone;
   p->getPacketContents(CONVERSION_NO,
                        inSender,
                        inRec,
                        dataLength,
                        data);
   senderPhone = inSender;
   recipientPhone = inRec;
   delete [] data;

   if ( senderPhone == NULL )
      senderPhone = "NULL";
   if ( recipientPhone == NULL )
      recipientPhone = "NULL";
   
   snprintf(packetInfo, Processor::c_maxPackInfo, "%s->%s, s=%u",
            senderPhone, recipientPhone, dataLength);
}

// This function is not a member of the class SMSProcessor.
// It's a kind of souped up macro
static inline void
makePacketInfo(char* packetInfo,
               SMSListenRequestPacket* p)
{
   const char* phoneNumber = NULL;
   char* phone = (char*)phoneNumber;
   p->getPhoneNumber(phone);
   phoneNumber = phone;

   if ( phoneNumber == NULL )
      phoneNumber = "NULL";

   snprintf(packetInfo, Processor::c_maxPackInfo, "%s",
            phoneNumber);
}

Packet* SMSProcessor::handleRequestPacket( const RequestPacket& pack,
                                           char* packetInfo )
{
   

   // We can handle SMSSendRequestPackets and SMSListenRequestPackets
   switch ( pack.getSubType() ) {
      case Packet::PACKETTYPE_SMSREQUEST: {
         SMSSendRequestPacket* smsPack = static_cast<SMSSendRequestPacket*>(
                                         pack.getClone(true));
         makePacketInfo(packetInfo, smsPack);
         return m_smsTalker->sendSMS( smsPack );
         break;
      }
      default:
      {
         // Unknown packet
         return NULL;
      }         
   }
}


int SMSProcessor::getCurrentStatus()
{
   return 0;
}

void
SMSProcessor::SMSReceived(SMSSendRequestPacket* p)
{
   m_forwardThread->enqueue(p);
}
