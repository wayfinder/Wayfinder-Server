/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TransactionPacket.h"
#include "StringTable.h"


// /////////////////////////////////////////////////////////////////////
// TransactionRequestPacket
// /////////////////////////////////////////////////////////////////////


TransactionRequestPacket::TransactionRequestPacket(
   uint32 UIN, action_t action, 
   uint32 nbrTransactions  )
      : RequestPacket( MAX_PACKET_SIZE,
                       DEFAULT_PACKET_PRIO,
                       Packet::PACKETTYPE_TRANSACTION_REQUEST,
                       0,
                       0,
                       MAX_UINT32 )
{
   int pos = REQUEST_HEADER_SIZE;
   incWriteLong( pos, UIN );
   incWriteLong( pos, action );
   incWriteLong( pos, nbrTransactions );
   setLength( pos );
}


uint32 
TransactionRequestPacket::getUIN() const {
   return readLong( REQUEST_HEADER_SIZE );
}


TransactionRequestPacket::action_t 
TransactionRequestPacket::getAction() const {
   return action_t( readLong( REQUEST_HEADER_SIZE + 4 ) );
}


uint32
TransactionRequestPacket::getNbrTransactions() const {
   return readLong( REQUEST_HEADER_SIZE + 8 );
}


////////////////////////////////////////////////////////////////////////
// TransactionReplyPacket
////////////////////////////////////////////////////////////////////////


TransactionReplyPacket::TransactionReplyPacket( 
   const TransactionRequestPacket* req,
   StringCode status,
   uint32 nbrTransactions )
      : ReplyPacket( MAX_PACKET_SIZE,
                     Packet::PACKETTYPE_TRANSACTION_REPLY,
                     req,
                     status)
{
   int pos = REPLY_HEADER_SIZE;
   incWriteLong( pos, nbrTransactions );
   setLength( pos );
}


uint32 
TransactionReplyPacket::getNbrTransactions() const {
   return readLong( REPLY_HEADER_SIZE );
}


// /////////////////////////////////////////////////////////////////////
// TransactionDaysRequestPacket
// /////////////////////////////////////////////////////////////////////


TransactionDaysRequestPacket::TransactionDaysRequestPacket(
   uint32 UIN, bool check, 
   int32 nbrTransactionDays )
      : RequestPacket( REQUEST_HEADER_SIZE + 4 + 4 + 1,
                       DEFAULT_PACKET_PRIO,
                       Packet::PACKETTYPE_TRANSACTION_DAYS_REQUEST,
                       0,
                       0,
                       MAX_UINT32 )
{
   int pos = REQUEST_HEADER_SIZE;
   incWriteLong( pos, UIN );
   incWriteLong( pos, nbrTransactionDays );
   incWriteByte( pos, check );
   setLength( pos );
}


uint32 
TransactionDaysRequestPacket::getUIN() const {
   return readLong( REQUEST_HEADER_SIZE );
}


bool
TransactionDaysRequestPacket::getCheck() const {
   return readByte( REQUEST_HEADER_SIZE + 8 ) != 0;
}


int32
TransactionDaysRequestPacket::getNbrTransactionDays() const {
   return readLong( REQUEST_HEADER_SIZE + 4 );
}


////////////////////////////////////////////////////////////////////////
// TransactionDaysReplyPacket
////////////////////////////////////////////////////////////////////////


TransactionDaysReplyPacket::TransactionDaysReplyPacket( 
   const TransactionDaysRequestPacket* req,
   StringCode status,
   uint32 nbrTransactionDays,
   uint32 currStartTime )
      : ReplyPacket( REPLY_HEADER_SIZE + 4 + 4,
                     Packet::PACKETTYPE_TRANSACTION_DAYS_REPLY,
                     req,
                     status)
{
   int pos = REPLY_HEADER_SIZE;
   incWriteLong( pos, nbrTransactionDays );
   incWriteLong( pos, currStartTime );
   setLength( pos );
}


uint32 
TransactionDaysReplyPacket::getNbrTransactionDays() const {
   return readLong( REPLY_HEADER_SIZE );
}


uint32 
TransactionDaysReplyPacket::getCurrStartTime() const {
   return readLong( REPLY_HEADER_SIZE + 4 );
}
