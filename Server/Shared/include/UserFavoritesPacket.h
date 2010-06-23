/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERFAVORITESPACKET_H
#define USERFAVORITESPACKET_H

#include "config.h"
#include "UserPacket.h"

/**
 *  This request packet handles synchronization of and adding/deleting
 *  User Favorites
 *
 *  After the usual user request header the packet contains 4 variable
 *  length lists of favorites that looks like this:
 *  \begin{tabular}{lll}
 *  Position & Size   & \\ \hline
 *  USER_REQUEST_HEADER_SIZE  & 1 bytes  & Number elements sync list\\
 *  ...                     & n * 4 bytes & ID of favorites to sync\\
 *  ...                     & 1 bytes  & Number elements add list\\
 *  ...                     & n * x bytes & Elements of add list\\
 *  ...                     & 1 bytes  & Number elements delete list\\
 *  ...                     & n * 4 bytes & ID of favorites to sync\\
 *  ...                     & 1 bytes  & Number elements auto dest list\\
 *  ...                     & n * x bytes & User Favorites of auto dest list\\
 *  \end{tabular}
 *  Each element (favorite) looks like this:
 *  \begin{tabular}{lll}
 *  Position & Size   & \\ \hline
 *  0        & 4 bytes & favorite ID (uint32)
 *  4        & 4 bytes & latitute (int32)
 *  8        & 4 bytes & longitude (int32)
 *  12       & 4 bytes & longitude (int32)
 *  x        & n bytes & name (nul terminated string)
 *  x        & n bytes & short name (nul terminated string)
 *  x        & n bytes & description (nul terminated string)
 *  x        & n bytes & category (nul terminated string)
 *  x        & n bytes & map icon name (nul terminated string)
 **/
class UserFavoritesRequestPacket : public UserRequestPacket 
{
   public:
      /** 
        *   Constructor
        *   @param   packetID The packet ID.
        *   @param   reqID    The request ID.
        *   @param   UIN      The User ID whose favorites we will use
        */
      UserFavoritesRequestPacket(uint32 packetID, uint32 reqID, uint32 UIN);

};

/**
 *  This reply packet handles synhronization of and adding/deleting
 *  User Favorites
 *
 *  After the usual reply header the packet contains:
 *  \begin{tabular}{lll}
 *  Position & Size   & \\ \hline
 *  20       & x bytes & foo \\
 *  \end{tabular}
 *
 **/
class UserFavoritesReplyPacket : public UserReplyPacket 
{
   public:
      /** 
        *   Constructor
        *   @param   req    Pointer to the request packet we're the reply to
        */
      UserFavoritesReplyPacket(const UserFavoritesRequestPacket* req);

};

#endif // USERFAVORITESPACKET_H

