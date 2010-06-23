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

#include "SearchExpandItemPacket.h"
#include "IDPairVector.h"
#include "SearchMatch.h"

int
SearchExpandItemRequestPacket::
getItems(vector<pair<uint32, IDPair_t> >& result) const
{
   int pos = NBR_ITEMS_POS;
   int nbrItems = incReadLong(pos);
   for ( int i = 0; i < nbrItems; ++i ) {
      uint32 idx = incReadLong(pos);
      uint32 mapID = incReadLong(pos);
      uint32 itemID = incReadLong(pos);
      result.push_back(make_pair(idx, IDPair_t(mapID, itemID)));
   }
   return nbrItems;
}

uint32
SearchExpandItemRequestPacket::getUserData() const
{
   return readLong(USER_DEF_POS);
}

bool
SearchExpandItemRequestPacket::getExpand() const
{
   return readLong(EXPAND_FLAG_POS);
}

//-----------------------------------------------------------------
// SearchExpandItemReplyPacket
//-----------------------------------------------------------------

int
SearchExpandItemReplyPacket::
calcPacketSize(const vector<pair<uint32, IDPair_t> >& translated)
{
   // This is not right anymore
   return ITEMS_START_POS + translated.size() * 12;
}

int
SearchExpandItemReplyPacket::
calcPacketSize(const vector<pair<uint32, OverviewMatch*> >& translated)
{
   // This is not right anymore
   return ITEMS_START_POS + translated.size() * 30 + 1024;
}

SearchExpandItemReplyPacket::
SearchExpandItemReplyPacket(const SearchExpandItemRequestPacket* req,
                            const vector<pair<uint32, IDPair_t> >& translated)
      : ReplyPacket( calcPacketSize(translated),
                     Packet::PACKETTYPE_SEARCHEXPANDITEMREPLY,
                     req,
                     StringTable::OK)
{
   int pos = USER_DEF_POS;
   incWriteLong(pos, req->getUserData());
   incWriteLong(pos, translated.size());
   for ( uint32 i = 0; i < translated.size(); ++i ) {
      // Update position
      setLength(pos);      
      if ( updateSize( 1024, 1024+getBufSize()) ) {
         mc2dbg8 << "[SEIP]: Resized packet to " << getBufSize()
                 << " bytes" << endl;
      }
      incWriteLong(pos, translated[i].first);
      // From now on we will have overview matches here.
      OverviewMatch match(translated[i].second);
      match.save(this, pos);
   }
   setLength(pos);
}

SearchExpandItemReplyPacket::
SearchExpandItemReplyPacket(const SearchExpandItemRequestPacket* req,
                            const vector<pair<uint32, OverviewMatch*> >&
                            translated)
      : ReplyPacket( calcPacketSize(translated),
                     Packet::PACKETTYPE_SEARCHEXPANDITEMREPLY,
                     req,
                     StringTable::OK)
{
   int pos = USER_DEF_POS;
   incWriteLong(pos, req->getUserData());
   incWriteLong(pos, translated.size());
   for ( uint32 i = 0; i < translated.size(); ++i ) {
      // Update position
      setLength(pos);      
      if ( updateSize( 1024, 1024+getBufSize()) ) {
         mc2dbg8 << "[SEIP]: Resized packet to " << getBufSize()
                 << " bytes" << endl;
      }
      incWriteLong(pos, translated[i].first);
      // From now on we will have overview matches here.
      translated[i].second->save(this, pos);
   }
   setLength(pos);
}

int
SearchExpandItemReplyPacket::
getItems(multimap<uint32, OverviewMatch*>& expandedItems) const
{
   int pos = NBR_ITEMS_POS;
   int nbrItems = incReadLong(pos);
   for ( int i=0; i < nbrItems; ++i ) {
      uint32 index = incReadLong(pos);
      // Should be overview match or else...
      OverviewMatch* ov =
         static_cast<OverviewMatch*>(SearchMatch::createMatch(this, pos));
      expandedItems.insert(make_pair(index, ov));
   }
   return nbrItems;
}

int
SearchExpandItemReplyPacket::
getItems(multimap<uint32, IDPair_t>& expandedItems) const
{
   int pos = NBR_ITEMS_POS;
   int nbrItems = incReadLong(pos);
   for ( int i=0; i < nbrItems; ++i ) {
      uint32 index = incReadLong(pos);
      // Should be overview match or else...
      OverviewMatch* ov =
         static_cast<OverviewMatch*>(SearchMatch::createMatch(this, pos));
      expandedItems.insert(make_pair(index, ov->getID()));
      delete ov;
   }
   return nbrItems;
}

uint32
SearchExpandItemReplyPacket::getUserData() const
{
   return readLong(USER_DEF_POS);
}
