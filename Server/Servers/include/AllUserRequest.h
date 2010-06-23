/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ALLUSERREQUEST_H
#define ALLUSERREQUEST_H

#include "Request.h"
#include "PacketContainer.h"
#include "UserPacket.h"

/** 
 *    A request that returns all users from a specific request.
 *
 */
class AllUserRequest : public Request {
   public:
      /**
       *    Create an AllUserRequest.
       *
       *    @param reqID Unique request ID.
       *    @param p     Packet to be sent by request containing the 
       *                 search criteria.
       */
      AllUserRequest( uint16 reqID,
                      FindUserRequestPacket* p );

      /** 
       *    Get the next packet to return from this request.
       *    @return The next valid packet or NULL if no valid packet 
       *            available.
       */
      PacketContainer* getNextPacket();

      /** 
       *    Process a packet that is returned from the modules.
       *    @pack The next packet to be processed.
       */
      void processPacket( PacketContainer* pack );

      /**
       *    Get the answer from this request.
       *    @return The answer to the request.
       */
      PacketContainer* getAnswer();

      /**
       *    Get the user reply packets.
       *    @return The user reply or NULL if not valid.
       */
      inline GetUserDataReplyPacket** getUserReplyPacket();

      /**
       *    Get the number of packets that will be returned in the
       *    getUserReplyPacket()-method.
       *    @return The number of user reply packets.
       */
      inline int32 getNbrUserReplyPacket();

   private:
      /**
       *    Get the next packetcontainer for the users.
       *    @param p   The incoming packet to be stored in the 
       *               GetUserDataReplyPacket vector. If p is NULL the 
       *               vector is created.
       *    @return The next PacketContainer containg a 
       *            GetUserDataRequestPacket. Returns NULL when we are done.
       */
      PacketContainer* getNextUserPacketContainer( GetUserDataReplyPacket* p );
      
      /** 
       *    The possible states for this request.
       */
      enum AllUserRequest_t{
         /// The find-user packet is send
         FIND_USER_SENT = 0,

         /// Send user request packet.
         USER_REQUEST_SENT = 1
      } m_state;

      /**
       *    Temporarily stores the FindUserReplyPacket.
       */
      FindUserReplyPacket* m_userReply;
      
      /** 
       *    A vector containing a pointer to the users. To be returned 
       *    by the request.
       */
      GetUserDataReplyPacket** m_userVect;

      /**
       *    A list of all the IDs of the user to get information about.
       */
      uint32* m_UI; 

      /**
       *    Used to index the m_userVect.
       */ 
      uint32 m_userIndex;
      
      /**
       *    The current packet ID.
       */
      uint32 m_currentPacketID;

      /** 
       *    The next request to be sent.
       */ 
      PacketContainer* m_request;

      /**
       *    The answer. {\it {\bf NB!} For now it is always NULL.}
       */
      PacketContainer* m_answer;
};

// ========================================================================
//                                       Implementation of inline methods =

GetUserDataReplyPacket**
AllUserRequest::getUserReplyPacket()
{
   return m_userVect;
}

int32 
AllUserRequest::getNbrUserReplyPacket()
{
   if( m_userReply != NULL )
      return m_userReply->getNbrUsers();
   else
      return 0;
}

#endif

