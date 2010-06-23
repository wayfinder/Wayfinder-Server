/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SMSPACKET_H
#define SMSPACKET_H

#include "Packet.h"
#include "Types.h"
#include "config.h"

// No conversion  - send the SMS as it is
#define CONVERSION_NO    0
// Text conversion - convert the SMS to the GSM character set
#define CONVERSION_TEXT  1
// Define the size of this packet.
#define SMSPACKETLENGTH MAX_PACKET_SIZE

// Define the priority of the request
#define SMS_REQ_PRIO DEFAULT_PACKET_PRIO

/**
 *  A packet encaplsulating an SMS Message send request.
 *  Subtype == PACKETTYPE_SMSREQUEST
 *  After the usual header the request packet contains:
 *  \begin{tabular}{lll}
 *  Position & Size   & \\ \hline
 *  20       & 2 bytes & Data length \\
 *  22       & 1 byte  & SMS type flag \\
 *  23       & string  & Sender phone number \\
 *  x        & string  & Recipient phone number \\
 *  y        & Data length & Data, data format depends on SMS type flag \\
 *  \end{tabular}
 *
 *  It should be safe to turn an SMSSendRequest into an
 *  SMSSendReply by casting it to SMSSendReply and changing
 *  its type.
 *
 */
class SMSSendRequestPacket : public RequestPacket 
{
   public:

      enum smsType_t {
         // normal text message
         SMSTYPE_TEXT         = 0, 
         // normal text message, but data is in HEX representation, 
         // no conversion is done
         SMSTYPE_TEXT_HEX     = 1,
         // WAP Push, Service Indication, see Wap Forum specification 
         // WAP-167-ServiceInd-20010731-a
         SMSTYPE_WAP_PUSH_SI  = 2,
      };

      /**
        *   @name Create a SMSRequestPacket.
        */
      //@{
         /** 
           *   Full version
           *   @param   packetID The packet ID.
           *   @param   reqID    The request ID.
           */
         SMSSendRequestPacket(uint32 packetID, uint32 reqID,
                              uint32 origIP, uint16 origPort);   

         /** 
           *   Light version, without any parameters.
           */
         SMSSendRequestPacket();
      //@}

      /**
       *   Copies the data into this packet, either normal text message
       *   or hexadecimal text message
       *
       *   @param conversion     Convert the SMS from text or not.
       *   @param senderPhone    The phonenumber for the packet to be sent
       *                         from.
       *   @param recipientPhone The phonenumber to send to.
       *   @param dataLength     The length of the SMS data.
       *   @param data           A pointer to the SMS data.
       *   @param smsType        see SMSRequestPacket::smsType_t
       */
      void fillPacket(int conversion, const char* senderPhone, 
                      const char* recipientPhone, int dataLength, 
                      const byte* data,
                      smsType_t smsType = SMSTYPE_TEXT,
                      int smsNumber = -1,
                      int nbrParts = -1,
                      int partNbr = -1);
      
      /**
       *   Copies the data into this packet, WAP Push Service Indication
       *
       *   @param senderPhone    The phonenumber for the packet to be sent
       *                         from.
       *   @param recipientPhone The phonenumber to send to.
       *   @param href           The URI to use, UTF-8 encoded
       *   @param message        The message to use, UTF-8 encoded
       */
      void fillPacket(const char* senderPhone, 
                      const char* recipientPhone, 
                      const char* href,
                      const char* message);
     
      /**
       *   Get the type of the SMS
       */
      inline smsType_t getSMSType();

      /**
       *   Get the contents of the packet.
       *   Note that the pointer variables point into the packet
       *   where the data is, no copy is made except for the data.
       *   @param conversion   Set this to CONVERSION_TEXT if the SMS should
       *                       be converted from GSM- to ISO-text.
       *                       WARNING! Conversions other that CONVERSION_NO
       *                       actually changes the data of the packet, so
       *                       the data will be destroyed if converted twice.
       *   @param senderPhone  Points to the null-terminated string containing
       *                       the phonenumber of the sender of the SMS.
       *   @param recipientPhome The phonenumber of the recipient.
       *   @param dataLength     The length of the SMS data.
       *   @param data           A pointer to the actual SMS data. Must be
       *                         deleted by caller.
       */
      bool getPacketContents( int conversion,
                              char* &senderPhone,
                              char* &recipientPhone,
                              int &dataLength,
                              byte* &data );

      /**
       *   Get the contents of the packet.
       *   Note that the pointer variables point into the packet
       *   where the data is, no copy is made except for the data.
       *
       *   This function is mostly for internal use in the SMSModule.
       *
       *   @param senderPhone  Points to the null-terminated string containing
       *                       the phonenumber of the sender of the SMS.
       *   @param recipientPhome The phonenumber of the recipient.
       *   @param dataLength     The length of the SMS data.
       *   @param data           A pointer to the actual SMS data. Must be
       *                         deleted.
       *   @param smsNumber      A unique reference number for the concatenated
       *                         sms.
       *   @param nbrParts       The number of parts in the SMS.
       *   @param partNumber     The sequence number of this part.
       *   @param smsType        The type of the SMS
       */
      bool getPacketContents( int conversion,
                              char* &senderPhone,
                              char* &recipientPhone,
                              int &dataLength,
                              byte* &data,
                              int& smsNumber,
                              int& nbrParts,
                              int& partNbr);
      
      /**
       *   Get the contents of the packet, for a WAP Push SI
       *   Note that the pointer variables point into the packet
       *   where the data is, no copy is made.
       *
       *   This function is for internal use in the SMSModule.
       *
       *   @param senderPhone  Points to the null-terminated string containing
       *                       the phonenumber of the sender of the SMS.
       *   @param recipientPhone The phonenumber to send to.
       *   @param href           The URI to use, UTF-8 encoded
       *   @param message        The message to use, UTF-8 encoded
       */
      bool getPacketContents( char* &senderPhone,
                              char* &recipientPhone,
                              char* &href,
                              char* &message);
      
      /**
       *   Search through the phoneNumbers and serviceNames and see if it
       *   fits with the senderPhone in this packet. Starts with decoding
       *   the packet, so it isn't very fast.
       */
      bool findSenderPhoneOrService(int numberOfPhones,
                                    char** phoneNumbers,
                                    int numberOfServices,
                                    char** serviceNames);

      /**
        *   Print the phonenumbers of the sender and recipent and
        *   the data as a string into an allocated string.
        *
        *   @param   buffer   The pre allocated string where the result
        *                     is written.
        *   @param   maxLen   The maximum number of characters written into
        *                     buffer.
        *   @return  A pointer to buffer (the given prameter).
        */
      char* printSMS(char* buffer, int maxLen);
};

/**
  *   The sms reply. After the normal header this packet 
  *   contains (subType = PACKETTYPE_SMSREPLY):
  *   \begin{tabular}{lll}
  *      Pos      & Size    & \\ \hline
  *      18?      & 2 bytes  & Status of the SMS.\\
  *   \end{tabular}
  *
  */
class SMSSendReplyPacket : public ReplyPacket
{
   public:
      /**
        *   Creates an SMSSendReplyPacket as a reply to a given
        *   request.
        *   @param   p    The corresponding SMSSendRequestPacket.
        *   @param status Status (from StringTable).
        */
      SMSSendReplyPacket(const SMSSendRequestPacket* p, uint32 status);

};  

/**
 *    To listen to SMS messages. Do the following:
 *    \begin{enumerate}
 *    \item Open a TCP-port for listening
 *    \item Send an SMSListenRequest to the SMS Leader.
 *    \item If the request could be completed the most
 *          suitable SMS Module will connect to the port
 *          and IP given in the SMSListenRequestPacket and
 *          an SMSListenRequestReply with status OK is returned.
 *          If the request couldn't be completed an SMSListenReply
 *          with a status != OK is returned.
 *    \item When an SMS arrives at the SMSModule it is sent on the
 *          TCP socket.
 *
 *    The packet contains the following data:
 *
 *    \begin{tabular}{lll}
 *       Position & Size   & \\ \hline
 *       20       & 2 bytes & Contact port \\
 *       22       & string  & Phone number or service to listen to \\
 *    \end{tabular}
 */
class SMSListenRequestPacket : public RequestPacket
{
  public:
   /**
    *   Creates an SMSListenRequestPacket.
    *   @param packetID    The ID of the packet.
    *   @param reqID       The requestID
    *   @param origIP      The origin IP as usual
    *   @param origPort    The origin port.
    *   @param contactPort The port to contact.
    *   @param phoneNumber The phonenumber to listen to.
    */
   SMSListenRequestPacket(uint32 packetID, uint32 reqID,
                          uint32 origIP, uint16 origPort,
                          uint16 contactPort,
                          const char* phoneNumber);

   /** @return The port to contact on the listener */
   uint16 getContactPort() const;

   /** @return The IP of the listener. Same as origIP */
   uint32 getContactIP() const {
      return getOriginIP();
   };


   /**
    * @param phoneNumber A pointer that is changed to point into the
    *        phone number stored in the packet.
    * @return The length of the phoneNumber
    */
   int getPhoneNumber(char* &phoneNumber) const;

   /**
    *
    */
   int findPhoneNumber(int numberOfPhones, char** phones);
};


/**
 *   This packet is the answer to an SMSListenRequestPacket.
 *   Packet contents:
 *   \begin{tabular}{lll}
 *      Pos      & Size    & \\ \hline
 *      18?      & 2 bytes  & Status of the SMS.\\
 *   \end{tabular}
 */
class SMSListenReplyPacket : public ReplyPacket
{
  public:
   /**
    *   Creates an SMSListenReplyPacket as a reply to a given request.
    *   @param p The corresponding SMSListenRequestPacket.
    *   @param status The status of the request.
    */
   SMSListenReplyPacket(SMSListenRequestPacket*p, uint32 status);
};

inline SMSSendRequestPacket::smsType_t 
SMSSendRequestPacket::getSMSType() {
   return (smsType_t)readByte(REQUEST_HEADER_SIZE + 2);
}

#endif
