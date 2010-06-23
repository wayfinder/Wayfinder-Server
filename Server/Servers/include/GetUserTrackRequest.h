/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GETUSERTRACKREQUEST_H
#define GETUSERTRACKREQUEST_H

#include "config.h"
#include "Request.h"
#include "UserTrackElement.h"


/**
 *    Used to get tracking points for one user.
 *
 */
class GetUserTrackRequest : public RequestWithStatus {
   public:
      /**
       *    Create a new, empty request for one user.
       *    @param reqID ID of this request.
       *    @param UIN   The user identification number.  
       */
      GetUserTrackRequest(uint32 reqID, uint32 UIN);

      /**
       *    Set time interval. The start time is the time when the 
       *    tracking points should start to be extracted. This implies 
       *    that start time should be less than current time! The optional 
       *    end time describes the end interval of the track points to 
       *    extract. If not set, all points from start time will be 
       *    returned.
       *
       *    @param startTime The lower part of the time interval.
       *    @param endTime   Optional parameter with the upper part 
       *                     of the time interval. Current time is
       *                     used if not set.
       */
      void setTimeInterval(uint32 startTime, uint32 endTime = MAX_UINT32);

      /**
       *    Set the maximum number of hits to return.
       *    @param n The maximum number of tracking points to return.
       */
      void setMaxNbrHits(uint32 n);

      /**
       *    Get a reference to a list with the elements in the reply.
       *    Might contain 0 elements if no track points are registered
       *    in the requested interval. Use getStatus to get the status of
       *    the request!
       *
       *    The returned list (refernce) is valid until this request
       *    is deleted.
       *    
       *    @return A list with the elements in the requested
       *            interval. 
       */
      const UserTrackElementsList& getResult();

      /**
       *    Get the next packet that will be sent to the user module.
       *    @return A packet container with the next packet to send to
       *            the user module.
       */
      PacketContainer* getNextPacket();

      /**
       *    Handle the reply from the modules.
       *    @param pack The packet that is sent from the mosules.
       */
      virtual void processPacket(PacketContainer* pack);
      
      /**
       *    Get the status of this request. StringTable::OK is
       *    returned if everything is alright!
       *    @return A string code with the status of this request.
       */
      StringTable::stringCode getStatus() const;
      
   private:
      enum {
         /// Initial stat (no packets have been created)
         INIT,

         /// 
         GET_USER_TRACK_PACKET,
         DONE,
         ERROR
      } m_state;

      /**
       *    The list with elements to send to the User Module.
       */
      uint32 m_startTime;
      uint32 m_endTime;
      uint32 m_maxNbrHits;
      uint32 m_UIN;

      UserTrackElementsList m_resultElements;
};

#endif

