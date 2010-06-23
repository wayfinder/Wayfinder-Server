/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MATCHINFOPACKET_H
#define MATCHINFOPACKET_H

#include "config.h"

#include<vector>

#include "Packet.h"
#include "MC2Coordinate.h"
#include "LangTypes.h"


class IDPair_t;
class SearchMatch;
class VanillaMatch;
class VanillaStreetMatch;
class VanillaRegionMatch;

/**
 *   Packet for looking up house numbers and coordinates
 *   for searchmatches. Currently only needed for
 *   VanillaStreetMatches.
 */
class MatchInfoRequestPacket : public RequestPacket {
  public:
   
   /**
    *   Creates a new MatchInfoRequestPacket.
    *   Adds the VanillaStreetMatches that have the correct
    *   mapIDs to the packet and ignores the other ones.
    *   @param matches     Vector to pick the matches from.
    *   @param reqLang     Requested language.
    *   @param regionTypes Types of regions to add.
    *   @param mapID       MapID to send the packet to.
    *   @param packetID    Packet ID.
    *   @param reqID       Request ID.
    *   @param userData    User data will be copied by reply.
    */
   MatchInfoRequestPacket(const vector<VanillaMatch*>& matches,
                          uint32 regionTypes,
                          LangTypes::language_t reqLang,
                          uint32 mapID,
                          uint32 packetID = MAX_UINT32,
                          uint32 reqID    = MAX_UINT32,
                          uint32 userData = MAX_UINT32);

   /**
    *   Creates VanillaStreetMatches from the packet and puts
    *   them into the packet.
    *   @param streets Vector to put the created matches in.
    *                  These must be deleted by the caller later.
    *   @return Number of matches put in vector.
    */
   int getStreetMatches(vector<VanillaStreetMatch*>& streets) const;
   
   /**
    *   Returns user defined data.
    */
   uint32 getUserData() const;

   /**
    *   Returns the number of items in the packet.
    */
   uint32 getNbrItems() const;

   /**
    *   Returns the types of regions that should be returned
    *   in the answer.
    */
   uint32 getRegionSearchTypes() const;
   
   /**
    *   Returns the language wanted by the user.
    */
   LangTypes::language_t getRequestedLanguage() const;

private:

   /**
    *   Calculates the size needed for the packet.
    */
   static int calcPacketSize(const vector<VanillaMatch*>& matches);
   
   /**  The position of the user definded data. */
   static const int USER_DEF_POS     = REQUEST_HEADER_SIZE;   

   /**  The position of the requested language. */
   static const int LANGUAGE_POS     = USER_DEF_POS + 4;
   
   /**  The position of the rquested region types. */
   static const int REGION_TYPES_POS = LANGUAGE_POS + 4;

   /**  The position of the number of items */
   static const int NBR_ITEMS_POS    = REGION_TYPES_POS + 4;

   /**  The position where the item ids start */
   static const int ITEMS_START_POS   = NBR_ITEMS_POS + 4;
   
};


/**
 *   Reply to the MatchInfoRequestPacket.
 */
class MatchInfoReplyPacket : public ReplyPacket {
public:
   /**
    *    Creates a reply to a SearchExpandItemRequestPacket.
    *    (To be used by the module).
    *    @param req     The request packet.
    *    @param streets Streets with house numbers and regions filled
    *                   in.
    */
   MatchInfoReplyPacket(const MatchInfoRequestPacket* req,
                        const vector<VanillaStreetMatch*>& infos);

   /**
    *    Adds information about housenumbers, offsets etc to the
    *    oldMatches if the id is correct and there is more info
    *    in the match in the packet than in the oldMatches.
    *    @param oldMatches All the matches of the request.
    *    @return Number of updated matches.
    */
   int fixupMatches(vector<VanillaMatch*>& oldMatches) const;

   /**
    *   Returns user defined data.
    */
   uint32 getUserData() const;

   /**
    *   Returns the mapID.
    */
   uint32 getMapID() const;
   
   /**
    *   Returns the number of items in the packet.
    */
   uint32 getNbrItems() const;

private:

   /** Calculates the expected size of the packet. */
   static int calcPacketSize(const vector<VanillaStreetMatch*>& infos);

   /** Adds one VanillaStreetMatch to the packet */
   inline void incWriteStreet(int& pos,
                              const VanillaStreetMatch* street);

   /** Adds one region to the packet */
   inline void incWriteRegion(int& pos,
                              const SearchMatch* region);

   /** Reads one streetMatch from the packet */
   inline VanillaStreetMatch* incReadStreet(int& pos) const;

   /** Read one region and its regions */
   inline VanillaRegionMatch* incReadRegion(int& pos) const;
   
   /** The position of the map id */
   static const int MAP_ID_POS      = REPLY_HEADER_SIZE;
   
    /**  The position of the user definded data. */
   static const int USER_DEF_POS    = MAP_ID_POS + 4;
   
   /**  The position of the number of items */
   static const int NBR_ITEMS_POS   = USER_DEF_POS + 4;

   /**  The position where the items start */
   static const int ITEMS_START_POS = NBR_ITEMS_POS + 4;
};

// - Implementation of template methods.


#endif
