/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SMSPacket.h"
#include "SMSConvUtil.h"

//
//  SMSSendRequest
//

SMSSendRequestPacket::SMSSendRequestPacket()
      : RequestPacket( SMSPACKETLENGTH,
                       SMS_REQ_PRIO,
                       Packet::PACKETTYPE_SMSREQUEST,
                       0, // packetID
                       0, // reqID
                       MAX_UINT32)
{
   fillPacket(CONVERSION_NO, "", "", 0, NULL);
}

SMSSendRequestPacket::SMSSendRequestPacket(uint32 packetID,
                                           uint32 reqID,
                                           uint32 origIP,
                                           uint16 origPort)
      : RequestPacket( SMSPACKETLENGTH,
                       SMS_REQ_PRIO,
                       Packet::PACKETTYPE_SMSREQUEST,
                       packetID,
                       reqID,
                       MAX_UINT32)
{
   setOriginIP(origIP);
   setOriginPort(origPort);
   fillPacket(CONVERSION_NO, "", "", 0, NULL);
}

void SMSSendRequestPacket::fillPacket(int conversion,
                                      const char* senderPhone,
                                      const char* recipientPhone,
                                      int dataLength,
                                      const byte* data,
                                      smsType_t smsType,
                                      int smsNumber,
                                      int nbrParts,
                                      int partNbr)
{
   int position = REQUEST_HEADER_SIZE;
   incWriteShort(position, dataLength); // May be rewritten later.
   incWriteByte(position, smsType);
   incWriteString(position, senderPhone);
   incWriteString(position, recipientPhone);
   // Check if this SMS should be converted
   if ( conversion == CONVERSION_TEXT ) {
      byte* tempBuf = new byte[dataLength*6];
      int outlen = SMSConvUtil::mc2ToGsm((char*)tempBuf, 
                                         (const char*)data, dataLength);
      for(int i=0; i < outlen; i++) {
         incWriteByte(position, tempBuf[i]);
      }
      // Rewrite data size.
      writeShort( REQUEST_HEADER_SIZE, outlen );
      delete [] tempBuf;
   } else {
      // No conversion - just write the data.
      for(int i=0; i < dataLength; i++)
         incWriteByte(position, data[i]);
   }
   if ( smsNumber != -1 ) {
      incWriteByte(position, smsNumber);
      incWriteByte(position, nbrParts);
      incWriteByte(position, partNbr);
   }
   setLength(position);
}

void SMSSendRequestPacket::fillPacket(const char* senderPhone,
                                      const char* recipientPhone,
                                      const char* href,
                                      const char* message)
{
   int position = REQUEST_HEADER_SIZE;
   incWriteShort(position, 0);
   incWriteByte(position, SMSTYPE_WAP_PUSH_SI);
   incWriteString(position, senderPhone);
   incWriteString(position, recipientPhone);
   incWriteString(position, href);
   incWriteString(position, message);
   setLength(position);
}
   
bool SMSSendRequestPacket::getPacketContents(int conversion,
                                             char* &senderPhone,
                                             char* &recipientPhone,
                                             int &dataLength,
                                             byte* &data)
{
   int position = REQUEST_HEADER_SIZE;
   dataLength = incReadShort(position);
   incReadByte(position); // smsType, currently not used here
   incReadString(position, senderPhone);
   incReadString(position, recipientPhone);
   data = getBuf() + position;
   char* tempBuf = new char[dataLength*6];
   // Check if we should convert the SMS
   if ( conversion == CONVERSION_TEXT ) {
      dataLength = 
        SMSConvUtil::gsmToMc2(tempBuf, (const char*)data, dataLength);
   } else {
      // Do not convert
      memcpy(tempBuf, data, dataLength);
   }
   // Swap
   data = reinterpret_cast<byte*>(tempBuf);
   return true;
}

bool
SMSSendRequestPacket::getPacketContents(int conversion,
                                        char* &senderPhone,
                                        char* &recipientPhone,
                                        int &dataLength,
                                        byte* &data,
                                        int& smsNumber,
                                        int& nbrParts,
                                        int& partNbr)
{
   int position = REQUEST_HEADER_SIZE;
   dataLength = incReadShort(position);
   incReadByte(position); // skip smsType
   incReadString(position, senderPhone);
   incReadString(position, recipientPhone);
   data = getBuf() + position;
   position += dataLength;
   char* tempBuf = new char[dataLength*6];
   // Check if we should convert the SMS
   if ( conversion == CONVERSION_TEXT ) {
      dataLength = 
        SMSConvUtil::gsmToMc2(tempBuf, (const char*)data, dataLength);
   } else {
      // Do not convert
      memcpy(tempBuf, data, dataLength);
   }
   // Swap
   data = reinterpret_cast<byte*>(tempBuf);
   if ( position < (int)getLength() ) {
      smsNumber = incReadByte(position);
      nbrParts  = incReadByte(position);
      partNbr   = incReadByte(position);
   } else {
      smsNumber = nbrParts = partNbr = -1;
   }
   return true;
}

bool
SMSSendRequestPacket::getPacketContents( char* &senderPhone,
                                         char* &recipientPhone,
                                         char* &href,
                                         char* &message)
{
   int position = REQUEST_HEADER_SIZE;
   incReadShort(position); // skip dataLength
   incReadByte(position); // skip smsType
   incReadString(position, senderPhone);
   incReadString(position, recipientPhone);
   incReadString(position, href);
   incReadString(position, message);
   return true;
}

bool SMSSendRequestPacket::findSenderPhoneOrService(int numberOfPhones,
                                                    char** phoneNumbers,
                                                    int numberOfServices,
                                                    char** serviceNames)
{
   int encodingType = 0;
   char* senderPhone = NULL;
   char* recipientPhone = NULL;
   int dataLength = 0;
   byte* data;
   getPacketContents(encodingType, senderPhone, recipientPhone,
                     dataLength, data);
   delete [] data;
   int i;   
   // Search for phonenumber
   for( i=0; i < numberOfPhones; i++) 
      if ( strcmp(senderPhone, phoneNumbers[i] ) == 0 )
         return true;
   // Search for service
   for( i=0; i < numberOfServices; i++)
      if ( strcmp(senderPhone, serviceNames[i] ) == 0 )
         return true;

   // We didn't return earlier so we couldn't find it.
   return false;   
}


char*
SMSSendRequestPacket::printSMS(char* buffer, int maxLen)
{
   // Get the parameters
   int encodingType = 0, dataLength;
   char* senderPhone;
   char* receiverPhone;
   byte* data;
   getPacketContents(encodingType, senderPhone, receiverPhone, 
                     dataLength, data);
   const char* senderText = "Sender phone: ";
   const char* receiverText = "Receiver phone: ";
   const char* dataText = "Data length: ";
   const char *msgDelimiter = "=== data delimiter ===\n";
   
   int phoneLength =
      strlen(senderText)+ strlen(receiverText) +
      strlen(senderPhone) + strlen(receiverPhone) + 
      strlen(dataText) +
      strlen(msgDelimiter) + 3 + 3;  // dataLength + 3 char's
   
   if (phoneLength < maxLen) {
         // Write the phonenumbers
      sprintf(buffer, "%s%s%c%s%s%c%s%i%c%s%c",
              senderText, senderPhone, '\n', 
              receiverText, receiverPhone, '\n', 
              dataText, dataLength, '\n', msgDelimiter, '\0');
      strncat( buffer, (char*) data, MIN(dataLength, maxLen-phoneLength));
      strncat( buffer, msgDelimiter, MIN( strlen(msgDelimiter),
                                          maxLen-strlen( buffer ) ) );
   } else {
      sprintf(buffer, "%c", '\0');
   }
   for (uint32 i = 0; i < strlen(buffer); i++) {
         // '@', 'Å' or 'å' in GSM-ascii
      if ((buffer[i] == 0x0e) ||
          (buffer[i] == 0x0f))
      {
         buffer[i] = 'X';
      }
   }

   delete [] data;
   
   return (buffer);
}


//
//  SMSReply
//

SMSSendReplyPacket::SMSSendReplyPacket(const SMSSendRequestPacket* p,
                                       uint32 status) :
      ReplyPacket(SMSPACKETLENGTH,
                  PACKETTYPE_SMSREPLY,
                  p,
                  status)
{
   // All is done in ReplyPacket
}

SMSListenRequestPacket::SMSListenRequestPacket(uint32 packetID,
                                               uint32 reqID,
                                               uint32 origIP,
                                               uint16 origPort,
                                               uint16 contactPort,
                                               const char* phoneNumber)
      : RequestPacket( SMSPACKETLENGTH,
                       SMS_REQ_PRIO,
                       Packet::PACKETTYPE_SMSLISTENREQUEST,
                       packetID,
                       reqID,
                       MAX_UINT32)
{
   int position = REQUEST_HEADER_SIZE;
   setOriginIP(origIP);
   setOriginPort(origPort);

   incWriteShort(position, contactPort);
   incWriteString(position, phoneNumber);
   setLength(position);
}


uint16
SMSListenRequestPacket::getContactPort() const
{
   int position = REQUEST_HEADER_SIZE;
   return incReadShort(position);
}


int
SMSListenRequestPacket::findPhoneNumber(int numberOfPhones, char** phones)
{
   char* myPhone = NULL;
   getPhoneNumber(myPhone);
   for(int i=0; i < numberOfPhones; i++)
      if ( strcmp(myPhone, phones[i] ) == 0 )
         return i;
   // We haven't found it
   return -1;
}

int
SMSListenRequestPacket::getPhoneNumber(char* &phoneNumber) const
{
   int position = REQUEST_HEADER_SIZE + 2;
   return incReadString(position, phoneNumber);
}


SMSListenReplyPacket::SMSListenReplyPacket(SMSListenRequestPacket*p,
                                           uint32 status)
      :       ReplyPacket(SMSPACKETLENGTH,
                          PACKETTYPE_SMSLISTENREPLY,
                          p,
                          status)
{
   // All is done in replyPacket.
}
