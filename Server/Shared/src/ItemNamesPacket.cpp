/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ItemNamesPacket.h"
#include "StringTable.h"

#define ITEMNAMESREQUESTPACKET_PRIO               (4)
#define ITEM_NAMES_REQUEST_PREFERED_LANGUAGES_POS (REQUEST_HEADER_SIZE)
#define ITEM_NAMES_REQUEST_NBR_ITEMS_POS          (REQUEST_HEADER_SIZE+4)
#define ITEM_NAMES_REQUEST_FIRST_ITEM_POS         (REQUEST_HEADER_SIZE+8)


ItemNamesRequestPacket::ItemNamesRequestPacket(uint32 mapID, 
                                               uint16 requestID, 
                                               uint16 packetID)
   : RequestPacket(MAX_PACKET_SIZE,
                   ITEMNAMESREQUESTPACKET_PRIO,
                   Packet::PACKETTYPE_ITEM_NAMES_REQUEST,
                   packetID,
                   requestID,
                   mapID)
{
   writeLong(ITEM_NAMES_REQUEST_PREFERED_LANGUAGES_POS, 0);
   writeLong(ITEM_NAMES_REQUEST_NBR_ITEMS_POS, 0);
   setLength(ITEM_NAMES_REQUEST_FIRST_ITEM_POS);
}

void 
ItemNamesRequestPacket::setPreferedLanguage(LangTypes::language_t t)
{
   writeLong(ITEM_NAMES_REQUEST_PREFERED_LANGUAGES_POS, uint32(t));
}

LangTypes::language_t 
ItemNamesRequestPacket::getPreferedLanguage() const
{
   return (static_cast<LangTypes::language_t>
                      (readLong(ITEM_NAMES_REQUEST_PREFERED_LANGUAGES_POS)));
}

void 
ItemNamesRequestPacket::addItem(uint32 itemID)
{
   mc2dbg4 << "ItemNamesReqPack::addItem(): Adding itemID = " << itemID 
           << " nbrItems=" << getNbrItems() << endl;
   uint32 n = getNbrItems();
   int pos = ITEM_NAMES_REQUEST_FIRST_ITEM_POS+n*4;
   incWriteLong(pos, itemID);
   writeLong(ITEM_NAMES_REQUEST_NBR_ITEMS_POS, n+1);
   setLength(pos);
}

uint32 
ItemNamesRequestPacket::getNbrItems() const
{
   return (readLong(ITEM_NAMES_REQUEST_NBR_ITEMS_POS));
}

uint32 
ItemNamesRequestPacket::getItem(uint32 i) const
{
   if (i<getNbrItems()) {
      return (readLong(ITEM_NAMES_REQUEST_FIRST_ITEM_POS + i*4));
   } else {
      return (MAX_UINT32);
   }
}



#define ITEM_NAMES_REPLY_MAPID_POS        (REPLY_HEADER_SIZE)
#define ITEM_NAMES_REPLY_NBR_NAMES_POS    (REPLY_HEADER_SIZE+4)
#define ITEM_NAMES_REPLY_FIRST_NAME_POS   (REPLY_HEADER_SIZE+8)
#define ITEM_NAMES_REPLY_MAX_FILL_SIZE    (MAX_PACKET_SIZE-128)

ItemNamesReplyPacket::ItemNamesReplyPacket(const ItemNamesRequestPacket* p)
   : ReplyPacket(MAX_PACKET_SIZE, Packet::PACKETTYPE_ITEM_NAMES_REPLY, 
                 p, StringTable::OK)
{
   writeLong(ITEM_NAMES_REPLY_MAPID_POS, p->getMapID()); // MapID
   writeLong(ITEM_NAMES_REPLY_NBR_NAMES_POS, 0); // Nbr items
   setLength(ITEM_NAMES_REPLY_FIRST_NAME_POS);
}

bool
ItemNamesReplyPacket::addName(uint32 itemID, const char* name)
{
   int pos = getLength();
   if (pos < ITEM_NAMES_REPLY_MAX_FILL_SIZE) {
      AlignUtility::alignLong(pos);
      incWriteLong(pos, itemID);
      incWriteString(pos, name);
      setLength(pos);

      // Update nbrNames
      writeLong(ITEM_NAMES_REPLY_NBR_NAMES_POS, getNbrNames()+1);
      return (true);
   } else {
      return (false);
   }
}


uint32 
ItemNamesReplyPacket::getMapID() const
{
   return (readLong(ITEM_NAMES_REPLY_MAPID_POS));
}

uint32 
ItemNamesReplyPacket::getNbrNames() const
{
   return (readLong(ITEM_NAMES_REPLY_NBR_NAMES_POS));
}

int
ItemNamesReplyPacket::getFirstNamePosition() const
{
   return (ITEM_NAMES_REPLY_FIRST_NAME_POS);
}

const char* 
ItemNamesReplyPacket::getNextName(int& pos, uint32& itemID) const
{
   if (pos < int(getLength())) {
      AlignUtility::alignLong(pos);
      itemID = incReadLong(pos);
      char* retVal;
      incReadString(pos, retVal);
      return (retVal);
   } else {
      return (NULL);
   }
}


