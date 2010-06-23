/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GetUserTrackPacket.h"

#define GETUSERTRACK_REQUEST_LOWER_INTERVAL   (USER_REQUEST_HEADER_SIZE)
#define GETUSERTRACK_REQUEST_HIGHER_INTERVAL  (USER_REQUEST_HEADER_SIZE + 4)
#define GETUSERTRACK_REQUEST_MAXNBRHITS       (USER_REQUEST_HEADER_SIZE + 8)


GetUserTrackRequestPacket::GetUserTrackRequestPacket(uint16 packetID, 
                                                   uint16 reqID, 
                                                   uint32 UIN)
   : UserRequestPacket(MAX_PACKET_SIZE,
                       Packet::PACKETTYPE_GETUSERTRACK_REQUEST,
                       packetID,
                       reqID,
                       UIN)
{
   writeLong(GETUSERTRACK_REQUEST_LOWER_INTERVAL, 0);
   writeLong(GETUSERTRACK_REQUEST_HIGHER_INTERVAL, 0);
   writeLong(GETUSERTRACK_REQUEST_MAXNBRHITS, 0);
   setLength(GETUSERTRACK_REQUEST_MAXNBRHITS + 4);
}

void 
GetUserTrackRequestPacket::setInterval(uint32 lowInterval, uint32 highInterval)
{
   writeLong(GETUSERTRACK_REQUEST_LOWER_INTERVAL, lowInterval);
   writeLong(GETUSERTRACK_REQUEST_HIGHER_INTERVAL, highInterval);
}

void 
GetUserTrackRequestPacket::setMaxNbrHits(uint32 maxNbrHits)
{
   writeLong(GETUSERTRACK_REQUEST_MAXNBRHITS, maxNbrHits);
}

uint32 
GetUserTrackRequestPacket::getLowerInterval() const
{
   return readLong(GETUSERTRACK_REQUEST_LOWER_INTERVAL);
}

uint32 
GetUserTrackRequestPacket::getHigherInterval() const
{
   return readLong(GETUSERTRACK_REQUEST_HIGHER_INTERVAL);
}

uint32 
GetUserTrackRequestPacket::getMaxNbrHits() const
{
   return readLong(GETUSERTRACK_REQUEST_MAXNBRHITS);
}



// ========================================================================
//                                                 GetUserTrackReplyPacket =

#define GETUSERTRACK_REPLY_NBRELM_POS (USER_REPLY_HEADER_SIZE)

GetUserTrackReplyPacket::GetUserTrackReplyPacket(const GetUserTrackRequestPacket* req)
   : UserReplyPacket(MAX_PACKET_SIZE,
                     Packet::PACKETTYPE_GETUSERTRACK_REPLY, 
                     req)
{
   writeLong(GETUSERTRACK_REPLY_NBRELM_POS, 0);
   setLength(GETUSERTRACK_REPLY_NBRELM_POS + 4);
}

void
GetUserTrackReplyPacket::setUserTrackElements(UserTrackElementsList& elements)
{
   int pos = GETUSERTRACK_REPLY_NBRELM_POS;
   elements.store(this, pos);
   setLength(pos);
}

void
GetUserTrackReplyPacket::getUserTrackElements(UserTrackElementsList& elements) const
{
   int pos = GETUSERTRACK_REPLY_NBRELM_POS;
   elements.restore(this, pos);
   
}

