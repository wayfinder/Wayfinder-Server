/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DEBITPACKET_H
#define DEBITPACKET_H

#include "config.h"
#include "Packet.h"

// The prios and file
#define DEBIT_REQUEST_PRIO 1
#define DEBIT_FILENAME     "debit_log.txt"


/**
  *   Class describing the packet sent to the UserModule to charge a user
  *   for an operation.
  *
  *   After the general RequestPacket-header the format is:
  *   \begin{tabular}{rll}
  *      Position &  Size     & Description \\ \hline
  *      20       & 4 bytes   & UIN\\
  *      24       & 4 bytes   & Message id\\
  *      28       & 4 bytes   & Debit info\\
  *      32       & 4 bytes   & Date in unixtime\\
  *      36       & 4 bytes   & Kind of operation (full route, list etc.)\\
  *      40       & 4 bytes   & Size of operation (Nbr SMSes or Bytes)\\
  *      44       & 4 bytess  & nbrTransactions \\
  *      48       & ? bytes   & UserOriginID to debit as string\\
  *       ?       & ? bytes   & ServerID as string
  *       ?       & ? bytes   & Description of operation as string\\
  *      \ldots   &           & \\
  *   \end{tabular}
  *
  */
class DebitRequestPacket : public RequestPacket {
  public:
   /**
    *   Creates one DebitRequestpacket with the given parameters.
    *
    *   @param   packetID    The id of the packet.
    *   @param   requestID   The id of the request.
    *   @param   UIN         The users ID.
    *   @param   messageID   ID of message in mc2.
    *   @param   debInfo     MC2 debit value of this debit.
    *   @param   date        Timestamp in unixtime\\
    *   @param   sentSize    The amout sent, number SMSes or bytes.
    *   @param   operationType Kind of service performed.
    *   @param   UserOrigin  Origin of user, phoneNumber, IP.
    *   @param   ServerID    Phonenumber of smsserver or HTTPServer IP.
    *   @param   operationDescription What the server did.
    *   @param   nbrTransactions The number of transactions to withdraw
    *                            from user, default 0.
    *   @param   originIP    The sender's IP (default = 0, set later).
    *   @param   originPort  The port to reply to (default = 0, set later).
    */
   DebitRequestPacket( uint16 packetID, 
                       uint16 requestID,
                       uint32 UIN,
                       uint32 messageID,
                       uint32 debInfo,
                       uint32 date,
                       uint32 operationType,
                       uint32 sentSize,
                       const char* userOrigin,
                       const char* serverID,
                       const char* operationDescription,
                       uint32 nbrTransactions = 0,
                       uint32 originIP = 0,
                       uint16 originPort = 0 );

   
   /**
    *   @return the UIN
    */
   inline uint32 getUIN() const;
   
   
   /**
    *   @return the smsID
    */
   inline uint32 getMessageID() const;

   
   /**
    *   @return the number of sent sms
    */
   inline uint32 getSentSize() const;
   
   
   /**
    * Get the number of transactions to decrease the user's account by.
    * @return The number of transactions to withdraw.
    */
   inline uint32 getNbrTransactions() const;
   

   /**
    *   @return the debInfo
    */
   inline uint32 getDebitInfo() const;


   /**
    *   @return the timestamp
    */
   inline uint32 getDate() const;


   /**
    *   @return the kind of operation
    */
   inline uint32 getOperationType() const;


   /**
    *   @return  the user origin.
    */
   const char* getUserOrigin() const;

   
   /**
    *   @return   the servers ID, phonenumber or IP.
    */
   const char* getServerID() const;
   

   /**
    *   @return  the operationdescription
    */
   const char* getDescription() const;
   
};


/**
  *   Class describing the packet sent from the UserModule containing
  *   the answer to a DebitRequestPacket.
  *   Currently a DebitRequestPacket doesn't contain additional data
  *   it uses the status in the parentclass (ReplyPacket).
  *
  */
class DebitReplyPacket : public ReplyPacket {
   public:
   /**
    *   Creates a DebitReplyPacket in response to a DebitRequestPacket.
    *   Use the status member to check if the request was properly
    *   handled.
    *
    *   @param p the requestpacket to reply to
    *   @param status status on how the request was handled.
    */  
   DebitReplyPacket( const DebitRequestPacket* p, uint32 status );
};


////////////////////////////////////////////////////////////////////////
// Inline declarations
////////////////////////////////////////////////////////////////////////
uint32 
DebitRequestPacket::getUIN() const {
   return readLong( REQUEST_HEADER_SIZE );
}


uint32 
DebitRequestPacket::getMessageID() const {
   return readLong( REQUEST_HEADER_SIZE + 4 );
}


uint32 
DebitRequestPacket::getDebitInfo() const {
   return readLong( REQUEST_HEADER_SIZE + 8 );
}


uint32 
DebitRequestPacket::getDate() const {
   return readLong( REQUEST_HEADER_SIZE + 12 );
}


uint32 
DebitRequestPacket::getOperationType() const {
   return readLong( REQUEST_HEADER_SIZE + 16 );
}


uint32 
DebitRequestPacket::getSentSize() const {
   return readLong( REQUEST_HEADER_SIZE + 20 );
}


inline uint32 
DebitRequestPacket::getNbrTransactions() const {
   return readLong( REQUEST_HEADER_SIZE + 24 );
}


#endif
