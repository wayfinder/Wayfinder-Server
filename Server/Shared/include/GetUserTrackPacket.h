/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GETUSERTRACPACKET_H
#define GETUSERTRACPACKET_H

#include "config.h"
#include "UserPacket.h"
#include "UserTrackElement.h"

/**
 *    Used to get UserTrackElements that are stored in UM.
 *
 *    After the UserRequestPacket-header the format is 
 *    (X = USER_REQUEST_HEADER_SIZE):
 *    @packetdesc
 *       @row X    @sep 4 bytes @sep Start time of interval.      @endrow
 *       @row X+4  @sep 4 bytes @sep End time of interval.        @endrow
 *       @row X+8  @sep 4 bytes @sep Max nbr elements.            @endrow
 *    @endpacketdesc
 *
 */
class GetUserTrackRequestPacket : public UserRequestPacket {
   public:
      /**
       *
       */
      GetUserTrackRequestPacket(uint16 packetID, uint16 reqID, uint32 UIN);

      /**
       *    Set time interval for the elements to return.
       *    @param lowInterval   The lower limit of interval to return 
       *                         UserTrackElements in.
       *    @param highInterval  The upper limit of interval to return 
       *                         UserTrackElements in.
       */
      void setInterval(uint32 lowInterwal, uint32 highInterval);

      void setMaxNbrHits(uint32 maxNbrHits);

      uint32 getLowerInterval() const;
      uint32 getHigherInterval() const;
      uint32 getMaxNbrHits() const;
};

/**
 *    The reply to UserTrackRequestPacket. Status is set to aproperiate 
 *    value.
 *
 */
class GetUserTrackReplyPacket : public UserReplyPacket {
   public:
      GetUserTrackReplyPacket(const GetUserTrackRequestPacket* req);

      void setUserTrackElements(UserTrackElementsList& elements);
      void getUserTrackElements( UserTrackElementsList& elements) const;

};

#endif

