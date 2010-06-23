/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPAND_PACKET_H
#define EXPAND_PACKET_H

#include "config.h"
#include "Packet.h"

class ExpandRequestData;
class SearchReplyData;
class UserUser;
class LangTypes;
class UserRightsMapInfo;

/**
 *  Packet that requests all the Items that are belonged to a certain
 *  itemID.
 */
class ExpandRequestPacket : public RequestPacket {
   
  public:

   /**
    *  Calculates the size of the packet.
    */
   int calcPacketSize( const ExpandRequestData& data,
                       const UserUser* user );
   
   /**
    *  Constructor.
    *  @param data The ExpandRequestData.
    *  @param user The user.
    *  @param size The size of the data.
    */ 
   ExpandRequestPacket( const ExpandRequestData& data,
                        const UserUser* user );

   /**
    *  Returns the ExpandRequestData in the packet.
    *  @param data A reference to the ExpandRequestData.
    *  @param rights The user rights.
    */
   void get( ExpandRequestData& data,
             UserRightsMapInfo& rights ) const;
     
};

/**
 *  Reply to the ExpandRequestPacket.
 */
class ExpandReplyPacket: public ReplyPacket {

  public:
   /**
    *  Constructor.
    *  @param req The ExpandRequestPacket.
    *  @param res Reference to the SearchReplyData.
    */
   ExpandReplyPacket( const RequestPacket* req,
                      const SearchReplyData& res );

   /**
    *  Returns the SearchReplyData.
    *  @param res The SearchReplyData where the data is stored.
    */
   void get( SearchReplyData& res ) const;   
};

#endif
