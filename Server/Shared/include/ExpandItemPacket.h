/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDITEMPACKET_H
#define EXPANDITEMPACKET_H

#include "config.h"
#include "Packet.h"

/**
  *   Expand a Item into other items. This kind of questions are
  *   usefull for build up areas (get one or more street segment items),
  *   categories (get the companies in the category), streets (get
  *   the street segments that the street have) etc.
  *   <i><b>NB!</b> Currently only build up areas ar implemented!</i>
  *
  *   After the normal RequestPacket header this packet contains:
  *   @packetdesc
  *      @row +0 @sep 4 @sep Item ID         @endrow
  *      @row +4 @sep 4 @sep Expansion type  @endrow
  *   @endpacketdesc
  *
  *   @see     ExpandItemReplyPacket
  */
class ExpandItemRequestPacket : public RequestPacket {
   public:
      /**
        *   Expansion types.
        *   {\it NB! The types in this enum will increase ;-)}
        */
      enum expansion_t {
         bestMatch = 0
      };
   
      /**
        *   Default constructor.
        */
      ExpandItemRequestPacket();

      /**
        *   Constuctor that sets the IDs in the packet.
        */
      ExpandItemRequestPacket(uint32 packetID, uint32 reqID, uint32 mapID);

      /**
        *   Set the ID of the item that should be expand.
        *   @param   itemID   The ID of the item to expand.
        */
      void setItemID(uint32 itemID);

      /**
        *   @return  ID of the item that should be expanded.
        */
      uint32 getItemID() const;

      /**
        *   Set the wanted type of expansion.
        *   @param   type  Expansiontype.
        */
      void setExpansionType(expansion_t type);

      /**
        *   @return  The wanted type of expansion.
        */
      expansion_t getExpansionType() const;

   private:
      /**
        *   Reset the packet.
        */
      void initEmptyPacket();

};

/**
  *   The reply to a ExpandItemRequestPacket.
  *
  *   @see     ExpandItemRequestPacket
  */
class ExpandItemReplyPacket : public ReplyPacket {
   public:
      /**
        *   Create an empty ExpandItemReplyPacket.
        */
      ExpandItemReplyPacket();

      /**
        *   Create an ExpandItemReplyPacket from the corresponding
        *   request packet.
        */
      ExpandItemReplyPacket(const ExpandItemRequestPacket* req);

      /**
        *   Add one item ID to the reply packet.
        */
      void addItem(uint32 itemID);

      /**
        *   Get one item in the reply. Valid values for i is 
        *   0 <= i < getNbrItems().
        *   @param   i  The cardinal number of the wanted item.
        *   @return  ID of the i:th item. MAX_UINT32 is return upon error.
        */
      uint32 getItem(uint32 i) const;

      /**
        *   Get the number of items in this packet.
        */
      uint32 getNbrItems() const;

   private:
      /**
        *   Initiate the packet.
        */
      void initEmptyPacket();

};

#endif

