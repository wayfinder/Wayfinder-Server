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

#include "UserFavoritesPacket.h"
#include "UserFavoritesRequest.h"

UserFavoritesRequest::UserFavoritesRequest(uint16 reqID, uint32 UIN)
   : RequestWithStatus(reqID)
{
   m_reqPacket = new UserFavoritesRequestPacket(getNextPacketID(), reqID, UIN);
   m_replyPacketContainer = NULL;
   m_state = INIT;
}

      
UserFavoritesRequest::~UserFavoritesRequest()
{
   delete m_reqPacket;
   delete m_replyPacketContainer;
   
}

void
UserFavoritesRequest::addFavSync(const uint32 favID)
{
   m_syncReqList.addFavorite(favID);
}

void
UserFavoritesRequest::addFavDelete(const uint32 favID)
{
   m_deleteReqList.addFavorite(favID);
}

void
UserFavoritesRequest::addFavNew(
   const int32 lat, const int32 lon,
   const char* name, const char* shortName,
   const char* description, const char* category,
   const char* mapIconName,
   const char* infoStr )
{
   m_addReqList.addFavorite( 0, lat, lon, name, shortName, description,
                             category, mapIconName, infoStr );
}


void
UserFavoritesRequest::addFavNew( UserFavorite* userFav ) {
   m_addReqList.addFavorite( userFav );
}


void
UserFavoritesRequest::reset()
{
   m_addIter = m_addReplyList.begin();
   m_deleteIter = m_deleteReplyList.begin();
}

const UserFavorite*
UserFavoritesRequest::getAddFav()
{
   if (m_addIter == m_addReplyList.end()) 
      return NULL;

   return *m_addIter++;
}

const UserFavorite*
UserFavoritesRequest::getDelFav()
{
   if (m_deleteIter == m_deleteReplyList.end()) 
      return NULL;

   return *m_deleteIter++;
   
}


void
UserFavoritesRequest::setNoSync()
{
   addFavSync(MAX_UINT32);
}

PacketContainer*
UserFavoritesRequest::getNextPacket()
{
   if (INIT == m_state) {
      // populate the request packet
      int pos = USER_REQUEST_HEADER_SIZE; 
      m_syncReqList.store(m_reqPacket, pos, true);
      m_addReqList.store(m_reqPacket, pos);
      m_deleteReqList.store(m_reqPacket, pos, true);
      m_reqPacket->setLength(pos);
      m_state = WAITING_FOR_REPLY;
      PacketContainer* pc = new PacketContainer(m_reqPacket, 0, 0, 
                                                         MODULE_TYPE_USER);
      m_reqPacket = NULL;
      return pc;
   } else
      return NULL;
}

PacketContainer*
UserFavoritesRequest::getAnswer()
{
   return NULL;
}

void
UserFavoritesRequest::processPacket(PacketContainer* pack)
{
   if (pack != NULL) {
      Packet* packet = pack->getPacket();
      if ((packet != NULL) && 
          (packet->getSubType() == Packet::PACKETTYPE_USERFAVORITES_REPLY) &&
          (static_cast<ReplyPacket*>(packet)->getStatus() == StringTable::OK)){
         m_replyPacket = static_cast<UserFavoritesReplyPacket*>(packet);
         m_replyPacketContainer = pack;
         DEBUG4(m_replyPacket->dump());
         int pos = USER_REPLY_HEADER_SIZE;
         m_addReplyList.restore(m_replyPacket, pos);
         m_deleteReplyList.restore(m_replyPacket, pos, true);
         reset();
         m_state = OK;
         m_done = true;
         return;
      }
   }
 
   // some kind of error ocurred
   mc2dbg4 << "UserFavoritesRequest::processPacket(): some kind of problem, "
              "setting state to ERROR" << endl;
   m_state = ERROR;
   m_done = true;
}

StringTable::stringCode
UserFavoritesRequest::getStatus() const
{
   if (OK == m_state)
      return StringTable::OK;

   if ((INIT == m_state) || (WAITING_FOR_REPLY == m_state))
      return StringTable::UNKNOWN;

   return StringTable::NOTOK;
}
