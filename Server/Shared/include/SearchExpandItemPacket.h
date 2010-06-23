/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHEXPANDITEMPACKET_H
#define SEARCHEXPANDITEMPACKET_H

#include "config.h"

#include<vector>
#include<map>

#include "Packet.h"

class IDPair_t;
class OverviewMatch;

/**
 *   Packet for expanding (overview)matches into one or more matches
 *   and translating the id:s to lower level ids.
 */
class SearchExpandItemRequestPacket : public RequestPacket {
  public:
   
   /**
    *   Creates a new SearchExpandItemRequestPacket for <b>expanding</b>
    *   the supplied matches. (To be used by server).
    *   Only writes the matches with the mapID of the packet.
    */
   template<class SEARCHMATCH>
   SearchExpandItemRequestPacket(const vector<SEARCHMATCH*>& matches,
                                 uint32 mapID,
                                 uint32 packetID = MAX_UINT32,
                                 uint32 reqID    = MAX_UINT32,
                                 uint32 userData = MAX_UINT32);

   /**
    *   Creates a new SearchExpandItemRequestPacket for <b>UNexpanding</b>
    *   the supplied matches. (To be used by server).
    *   Only writes the matches with the translated mapID of the packet.
    *   @param matches Matches to add.
    *   @param translationMap Map to use for checking which matches to add.
    *                         If a lower level map is found in the map and
    *                         the found value is == mapID the match will be
    *                         added.
    */
   template<class SEARCHMATCH>
   SearchExpandItemRequestPacket(const vector<SEARCHMATCH*>& matches,
                                 const map<uint32,uint32>& translationMap,
                                 uint32 mapID,
                                 uint32 packetID = MAX_UINT32,
                                 uint32 reqID    = MAX_UINT32,
                                 uint32 userData = MAX_UINT32);


   /**
    *   Returns the items in the request. (Used by module).
    */
   int getItems(vector<pair<uint32, IDPair_t> >& result) const;

   /**
    *   Returns user defined data.
    */
   uint32 getUserData() const;

   /**
    *   Returns true if we should expand.
    */
   bool getExpand() const;

private:

   /**
    *   Calculates the size needed for the packet.
    */
   template<class SEARCHMATCH>
   static int calcPacketSize(const vector<SEARCHMATCH*>& matches);
   
   /**  The position of the user definded data. */
   static const int USER_DEF_POS    = REQUEST_HEADER_SIZE;   

   /**  The position where the expand flag is written */
   static const int EXPAND_FLAG_POS = USER_DEF_POS + 4;
   
   /**  The position of the user definded data. */
   static const int NBR_ITEMS_POS   = EXPAND_FLAG_POS + 4;

   /**  The position where the item ids start */
   static const int ITEMS_START_POS = NBR_ITEMS_POS + 4;
   
};

/**
 *
 *
 */
class SearchExpandItemReplyPacket : public ReplyPacket {
public:
   /**
    *    Creates a reply to a SearchExpandItemRequestPacket.
    *    (To be used by the old module and removed when it is removed).
    *    @param req The request packet.
    *    @param translated vector containing the original index in first
    *                      and the translated id in second. There can be
    *                      more than one entry with the same original id if
    *                      the item is expanded as well as translated.
    */
   SearchExpandItemReplyPacket(const SearchExpandItemRequestPacket* req,
                               const
                               vector<pair<uint32, IDPair_t> >& translated);

   /**
    *    Creates a reply to a SearchExpandItemRequestPacket.
    *    (To be used by the module).
    *    @param req The request packet.
    *    @param translated vector containing the original index in first
    *                      and the translated id in second. There can be
    *                      more than one entry with the same original id if
    *                      the item is expanded as well as translated.
    *                      The OverviewMatch contains the radius of the match.
    */
   SearchExpandItemReplyPacket(const SearchExpandItemRequestPacket* req,
                               const
                               vector<pair<uint32, OverviewMatch*> >&
                               translated);

   /**
    *    Returns the expanded items.
    *    @param expandedItems multimap with index in request-vector in first
    *                         and expanded item in second.
    */
   int getItems( multimap<uint32, OverviewMatch*>& expandedItems) const;

   /**
    *    Returns the expanded items.
    *    @param expandedItems multimap with index in request-vector in first
    *                         and expanded item in second.
    */
   int getItems( multimap<uint32, IDPair_t>& expandedItems) const;

   /**
    *   Returns user defined data.
    */
   uint32 getUserData() const;

private:
    /**  The position of the user definded data. */
   static const int USER_DEF_POS    = REPLY_HEADER_SIZE;   
   
   /**  The position of the user definded data. */
   static const int NBR_ITEMS_POS   = USER_DEF_POS + 4;

   /**  The position where the items start */
   static const int ITEMS_START_POS = NBR_ITEMS_POS + 4;

   /**
    *   Calculates the number of bytes to allocate for the packet.
    */
   static int
       calcPacketSize(const vector<pair<uint32, IDPair_t> >& translated);
   
   /**
    *   Calculates the number of bytes to allocate for the packet.
    */
   static int
       calcPacketSize(const vector<pair<uint32, OverviewMatch*> >& translated);
};

// - Implementation of template methods.

template<class SEARCHMATCH>
int
SearchExpandItemRequestPacket::
calcPacketSize(const vector<SEARCHMATCH*>& matches)
{
   return ITEMS_START_POS + matches.size() * 12 + 4;
}

template<class SEARCHMATCH>
SearchExpandItemRequestPacket::
SearchExpandItemRequestPacket(const vector<SEARCHMATCH*>& matches,
                              uint32 mapID,
                              uint32 packetID,
                              uint32 reqID,
                              uint32 userData)
      : RequestPacket(  calcPacketSize(matches),
                        DEFAULT_PACKET_PRIO,
                        Packet::PACKETTYPE_SEARCHEXPANDITEMREQUEST,
                        packetID,
                        reqID,
                        mapID)
                       
{
   int pos = USER_DEF_POS;
   incWriteLong( pos, userData );
   incWriteLong( pos, true); // EXPAND
   incWriteLong( pos, 0 ); // NbrItems
   int nbrAdded = 0;
   for ( uint32 i = 0; i < matches.size(); ++i ) {
      if ( matches[i]->getMapID() == mapID ) {
         nbrAdded++;
         incWriteLong(pos, i);
         incWriteLong(pos, matches[i]->getMapID());
         incWriteLong(pos, matches[i]->getItemID());
      }
   }
   writeLong(NBR_ITEMS_POS, nbrAdded);
   
   setLength(pos);
}

template<class SEARCHMATCH>
SearchExpandItemRequestPacket::
SearchExpandItemRequestPacket(const vector<SEARCHMATCH*>& matches,
                              const map<uint32,uint32>& translationMap,
                              uint32 mapID,
                              uint32 packetID,
                              uint32 reqID,
                              uint32 userData)
      : RequestPacket(  calcPacketSize(matches),
                        DEFAULT_PACKET_PRIO,
                        Packet::PACKETTYPE_SEARCHEXPANDITEMREQUEST,
                        packetID,
                        reqID,
                        mapID)
                       
{
   int pos = USER_DEF_POS;
   incWriteLong( pos, userData );
   incWriteLong( pos, false ); // UNEXPAND
   incWriteLong( pos, matches.size() ); // Needs to be corrected after.
   int nbrAdded = 0;
   for ( uint32 i = 0; i < matches.size(); ++i ) {
      map<uint32, uint32>::const_iterator it =
         translationMap.find( matches[i]->getMapID());
      if ( it != translationMap.end() && it->second == mapID ) {
         nbrAdded++;
         incWriteLong(pos, i);
         incWriteLong(pos, matches[i]->getMapID());
         incWriteLong(pos, matches[i]->getItemID());
      }
   }
   writeLong(NBR_ITEMS_POS, nbrAdded);
   setLength(pos);
}


#endif
