/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AddUserTrackPacket.h"

#define ADDUSERTRACK_REQUEST_NBRELM_POS   (USER_REQUEST_HEADER_SIZE)


AddUserTrackRequestPacket::AddUserTrackRequestPacket(uint16 packetID, 
                                                   uint16 reqID, 
                                                   uint32 UIN)
   : UserRequestPacket(MAX_PACKET_SIZE,
                       PACKETTYPE_ADDUSERTRACK_REQUEST,
                       packetID,
                       reqID,
                       UIN)
{
   writeLong(ADDUSERTRACK_REQUEST_NBRELM_POS, 0);
   setLength(ADDUSERTRACK_REQUEST_NBRELM_POS + 4);
}

void
AddUserTrackRequestPacket::setUserTrackElements(
      UserTrackElementsList& elements)
{
   int pos = ADDUSERTRACK_REQUEST_NBRELM_POS;
   elements.store(this, pos);
   setLength(pos);
}

void
AddUserTrackRequestPacket::getUserTrackElements(
      UserTrackElementsList& elements) const
{
   int pos = ADDUSERTRACK_REQUEST_NBRELM_POS;
   elements.restore(this, pos);
   
}

// ========================================================================
//                                                 AddUserTrackReplyPacket =

#define ADDUSERTRACK_REPLY_SIZE   (USER_REPLY_HEADER_SIZE)

AddUserTrackReplyPacket::AddUserTrackReplyPacket(const AddUserTrackRequestPacket* req)
   : UserReplyPacket(ADDUSERTRACK_REPLY_SIZE, 
                     Packet::PACKETTYPE_ADDUSERTRACK_REPLY, 
                     req)
{
   setLength(ADDUSERTRACK_REPLY_SIZE);
}

