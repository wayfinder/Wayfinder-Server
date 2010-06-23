/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMNAMESPACKET_H
#define ITEMNAMESPACKET_H

#include "config.h"
#include "Packet.h"
#include "ItemTypes.h"
#include "LangTypes.h"

/**
 *    Send the ID of some items that are located on the same map to
 *    the map module to get their names.
 * 
 */
class ItemNamesRequestPacket : public RequestPacket {
   public:
      /**
       *    Create a new item-names request packet.
       *    @param mapID     The ID of the maps where the items are
       *                     located.
       *    @param requestID The ID of the request where this packet
       *                     are created.
       *    @param packetID  The ID of this packet.
       */
      ItemNamesRequestPacket(uint32 mapID, uint16 requestID, 
                             uint16 packetID);

      /**
       *    Set the prefered language of the returned strings.
       *    @param t    The prefered language.
       */
      void setPreferedLanguage(LangTypes::language_t);

      /**
       *    Get the prefered language of the strings to return.
       *    @return The prefered language.
       */
      LangTypes::language_t getPreferedLanguage() const;
   
      /**
       *    Add one item to this packet.
       *    @param itemID The ID of the item to look up the name for.
       */
      void addItem(uint32 itemID);

      /**
       *    Get the number of names to get the names for.
       *    @return The number of items in this packet.
       */
      uint32 getNbrItems() const;

      /**
       *    Get one of the items in this packet.
       *    @param i  The index of the item to return. Valid values are
       *              0 <= i < getNbrItems().
       *    @return The ID of item number i. MAX_UINt32 will be returned
       *            upon error.
       */
      uint32 getItem(uint32 i) const;
};


/**
 *    The reply that is send from the MapModule with the names of the
 *    items.
 * 
 */
class ItemNamesReplyPacket : public ReplyPacket {
   public:
      /**
       *    Create a new Item names request packet from the corresponding
       *    request packet.
       *    @param p    The question that was send to the MapModule.
       */
      ItemNamesReplyPacket(const ItemNamesRequestPacket* p);

      /**
       *    Get the ID of the map where the items in this reply are 
       *    located.
       *    @return The ID of the map where the items are located.
       */
      uint32 getMapID() const;

      /**
       *    Add one name to this packet.
       *    @param itemID  The ID of the item who have the name.
       *    @param name    The name of the item with ID = itemID.
       *    @return True if the item is added, false otherwise 
       *            (probably the packet is full).
       */
      bool addName(uint32 itemID, const char* name);

      /**
       *    Get the number of names in this packet.
       *    @return  The numberof names in this packet.
       */
      uint32 getNbrNames() const;

      /**
       *    Get the position of the fist name. This should be used
       *    in the getNextName-method. E.g.
       *    \begin{verbatim}
               int pos = p->getFirstNamePosition();
               for (int j=0; j<p->getNbrNames(); j++) {
                  uint32 i;
                  const char* n = p->getNextName(pos, i);
                  ...
               }
            \end{verbatim}
       *    @return The position of the first name in this packet.
       */
      int getFirstNamePosition() const;

      /**
       *    Get the next name in this packet.
       *    @param pos     In/out-parameter that is used to keep track
       *                   of the position inside the packet. Updated
       *                   to the next string before returning.
       *    @param itemID  Outparameter that is set to the ID of the item
       *                   that have the name that is returned.
       *    @return A pointer to the name of item with ID = itemID. {\it
       *            {\bf NB!} This must no be deleted and must not be used
       *            when this packetis deleted}.
       */
      const char* getNextName(int& pos, uint32& itemID) const;
};

#endif

