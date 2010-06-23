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

#include "WFActivationPacket.h"
#include "UserData.h"


int
WFActivationRequestPacket::calcPacketSize( const char* activationCode,
                                           const char* userAgent,
                                           const char* userInput,
                                           const char* userName,
                                           const char* server )
{
   return REQUEST_HEADER_SIZE + strlen( activationCode ) + 1 +
      strlen( userAgent ) + 1 +
      strlen( userInput ) + 1 +
      strlen( userName ) + 1 +
      strlen( server ) + 1 +
      8 + 8;
}

WFActivationRequestPacket::
WFActivationRequestPacket( const char* activationCode,
                           WFActivationRequestPacket::whatToDo_t what,
                           uint32 ip, const char* userAgent,
                           const char* userInput,
                           const char* userName,
                           uint32 UIN,
                           const char* server,
                           uint32 packetID,
                           uint32 requestID )
      : RequestPacket( calcPacketSize( activationCode,
                                       userAgent,
                                       userInput,
                                       userName, 
                                       server ),
                       DEFAULT_PACKET_PRIO,
                       Packet::PACKETTYPE_WFACTIVATIONREQUEST,
                       packetID,
                       requestID,
                       MAX_UINT32) // Map id
                       
{
   int pos = WHAT_TO_DO_POS;
   // What
   incWriteLong( pos, what );
   // UIN
   incWriteLong( pos, UIN );
   // IP
   incWriteLong( pos, ip );
   // ActivationCode
   incWriteString( pos, activationCode );
   // userAgent
   incWriteString( pos, userAgent );
   // userInput
   incWriteString( pos, userInput );
   // userName
   incWriteString( pos, userName );
   // server
   incWriteString( pos, server );

   setLength( pos );
}

WFActivationRequestPacket::whatToDo_t
WFActivationRequestPacket::getWhatToDo() const
{
   return whatToDo_t( readLong( WHAT_TO_DO_POS ) );
}

const char*
WFActivationRequestPacket::getActivationCode() const
{
   int pos = ACTIVATION_CODE_POS;
   char* retVal;
   incReadString( pos, retVal );
   return retVal;
}


uint32 
WFActivationRequestPacket::getIP() const {
   return readLong( IP_POS );
}


const char* 
WFActivationRequestPacket::getUserAgent() const {
   int pos = ACTIVATION_CODE_POS;
   char* retVal;
   // ActivationCode
   incReadString( pos, retVal );
   // userAgent
   incReadString( pos, retVal );
   return retVal;
}


const char* 
WFActivationRequestPacket::getUserInput() const {
   int pos = ACTIVATION_CODE_POS;
   char* retVal;
   // ActivationCode
   incReadString( pos, retVal );
   // userAgent
   incReadString( pos, retVal );
   // userInput
   incReadString( pos, retVal );
   return retVal;
}


const char* 
WFActivationRequestPacket::getUserName() const {
   int pos = ACTIVATION_CODE_POS;
   char* retVal;
   // ActivationCode
   incReadString( pos, retVal );
   // userAgent
   incReadString( pos, retVal );
   // userInput
   incReadString( pos, retVal );
   // userName
   incReadString( pos, retVal ); 
   return retVal;
}


const char* 
WFActivationRequestPacket::getServer() const {
   int pos = ACTIVATION_CODE_POS;
   char* retVal;
   // ActivationCode
   incReadString( pos, retVal );
   // userAgent
   incReadString( pos, retVal );
   // userInput
   incReadString( pos, retVal );
   // userName
   incReadString( pos, retVal );
   // server
   incReadString( pos, retVal );
   return retVal;
}


uint32 
WFActivationRequestPacket::getUIN() const {
   return readLong( UIN_POS );   
}


//-----------------------------------------------------------------
// WFActivationReplyPacket
//-----------------------------------------------------------------


WFActivationReplyPacket::
WFActivationReplyPacket( const WFActivationRequestPacket* req,
                         WFResCodes::activation_res resultCode,
                         const char* rights, uint32 ownerUIN,
                         const char* server )
      : ReplyPacket( MAX_PACKET_SIZE,
                     Packet::PACKETTYPE_WFACTIVATIONREPLY,
                     req,
                     StringTable::OK )
{
   int pos = RESULT_CODE_POS;
   incWriteLong( pos, resultCode );
   incWriteLong( pos, ownerUIN );
   incWriteString( pos, rights );
   incWriteString( pos, server );
   setLength( pos );
}


WFResCodes::activation_res
WFActivationReplyPacket::getResultCode() const {
   return WFResCodes::activation_res( readLong( RESULT_CODE_POS ) );
}


uint32
WFActivationReplyPacket::getOwnerUIN() const {
   return readLong( UIN_POS );
}


const char*
WFActivationReplyPacket::getRights() const {
   int pos = RIGHTS_POS;

   return incReadString( pos );
}


const char*
WFActivationReplyPacket::getServer() const {
   int pos = RIGHTS_POS;

   // Skipp rights string
   incReadString( pos );

   return incReadString( pos );
}

