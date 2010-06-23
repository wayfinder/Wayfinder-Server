/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERFAVORITESREQUEST_H
#define USERFAVORITESREQUEST_H

#include "config.h"
#include "UserFavorites.h"
#include "Request.h"
#include "StringTable.h"

class UserFavoritesRequestPacket;
class UserFavoritesReplyPacket;

/**
 *  UserFavoritesRequest, used to sync/retrieve/add/delete favorites for a
 *  certain user.
 *
 */
class UserFavoritesRequest : public RequestWithStatus
{
   public:

      /** 
       *    Create a UserFavoritesRequest
       *    @param reqID      The unique request ID for this request.
       *    @param UIN        The user id associated with the favorites
       */
      UserFavoritesRequest(uint16 reqID, uint32 UIN);
      
      /**
       *    Delete this request, including allocated packet.
       */
      virtual ~UserFavoritesRequest();

      /**
       *   Add favorite ID to sync list
       *   @param favID The ID of the favorite
       */
      void addFavSync(const uint32 favID);

      /**
       *   Add favorite ID to delete list
       *   @param favID The ID of the favorite
       */
      void addFavDelete(const uint32 favID);

      /**
       *   Add new favorite to add list
       *   @param lat         The latitude
       *   @param lon         The longitude
       *   @param name        String containing the name
       *   @param shortName   String containing the short name
       *   @param description String containing the description
       *   @param category    String containing the category
       *   @param mapIconName String containing map icon name
       *   @param infoStr     The id-key-values in a string.
       */
      void addFavNew( const int32 lat, const int32 lon,
                      const char* name, const char* shortName,
                      const char* description, const char* category,
                      const char* mapIconName,
                      const char* infoStr );


      /**
       * Add new favorite to add list
       *
       * @param userFav The favorite to add, is now owned by this class.
       */
      void addFavNew( UserFavorite* userFav );


      /**
       *    Get the status of the request, returns stringcodes as defined
       *    in <tt>class StringTable</tt>. StringTable::OK indicates a
       *    successfull transaction. StringTable::UNKNOWN will be returned
       *    before the request placket has been sent/reply packet has been 
       *    received.
       *    @return status code
       */
      StringTable::stringCode getStatus() const;

      /**
       *   Reset the getAddFav() and getDelFav() functions.
       */
      void reset();

      /**
       *   Get the next favorite to add.
       *   @return Pointer to the next favorite, NULL if no more
       */
      const UserFavorite* getAddFav();

      /**
       *   Get the next favorite to delete.
       *   @return Pointer to the next favorite, NULL if no more
       */
      const UserFavorite* getDelFav();

      /**
       *   If called the request wont perform any syncing, only add/del
       *   for the user.
       */
      void setNoSync();

      /**
       *    Return a packet container with the next packet that should 
       *    be processed by the modules.
       *    @return  A packet container with the next packet that should
       *             be processed by the modules.
       */
      PacketContainer* getNextPacket();

      /**
       *    Returns NULL in this request, all packet handling is internal.
       *    Use getxxxx() to find out the status of the request.
       *    @return NULL pointer
       */
      PacketContainer* getAnswer();

      /**
       *    Take care of one packet that is processed by the modules.
       *    @param pack A packet container with a packet that have
       *                been processed by the modules.
       */
      void processPacket(PacketContainer* pack);

   private:

      /// add request list
      UserFavoritesList m_addReqList;

      /// delete request list
      UserFavoritesList m_deleteReqList;

      /// sync request list
      UserFavoritesList m_syncReqList;

      /// add reply list
      UserFavoritesList m_addReplyList;

      /// delete reply list
      UserFavoritesList m_deleteReplyList;

      /// iterator for m_addReplyList
      UserFavoritesList::iterator m_addIter;

      /// iterator for m_deleteReplyList
      UserFavoritesList::iterator m_deleteIter;

      /// our single request packet
      UserFavoritesRequestPacket* m_reqPacket;

      /// our single reply packet
      UserFavoritesReplyPacket* m_replyPacket;

      /// our single reply packet container to hold on to and delete later
      PacketContainer* m_replyPacketContainer;

      /// the internal states
      enum state_t {
          INIT,
          WAITING_FOR_REPLY,
          OK,
          ERROR
      };
            
      /// internal state variable
      state_t m_state;
};

#endif // USERFAVORITESREQUEST_H
