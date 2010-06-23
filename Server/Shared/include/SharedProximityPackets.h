/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SHARED_PROXIMITY_PACKETS_H
#define SHARED_PROXIMITY_PACKETS_H


#include "config.h"
#include "Types.h"
#include "Packet.h"
#include "StringTable.h"
#include "SearchTypes.h"


#define SHARED_PROXIMITY_REPLY_PRIO   0
#define SHARED_PROXIMITY_REQUEST_PRIO 0


/*
 * The type of answers to get from a Proximity Search.
 * More precisely the type of Reply packet Search Module sends.
 * \begin{itemize}
 *    \item PROXIMITY_SEARCH_ITEMS_NUMBERS The reply is a 
 *          ProximitySearchReplyPacket with the items as numbers.
 *    \item PROXIMITY_SEARCH_ITEMS_MATCHES The reply is a 
 *          VanillaSearchReplyPacket with the items as VanillaMatches.
 * \end{itemize}
 * 
 */
enum proximitySearchItemsType {
   PROXIMITY_SEARCH_ITEMS_NUMBERS = 1,
   PROXIMITY_SEARCH_ITEMS_MATCHES = 2,
};


/*
 * The content of a Position request packet can be.
 */
enum PositionRequestPacketContent {
   COVEREDIDSREQUEST_CONTENT_ITEMID  = 1,
   COVEREDIDSREQUEST_CONTENT_LAT_LON = 2,
   COVEREDIDSREQUEST_CONTENT_INVALID = 3,
};


/**
 *    Packet for proximity search from a position.
 *
 */
class ProximityPositionRequestPacket : public RequestPacket {
  public:
   /**
    * Packet for asking for hits in the proximity of a location.
    * If positioning from an item don't forget to set the mapID.
    *
    *   \\ After the normal header the request packet contains:
    *   \begin{tabular}{lll}
    *      Pos                   & Size     & Destription \\ \hline
    *      REQUEST_HEADER_SIZE   & 4 bytes  & latitude \\
    *      +4                    & 4 bytes  & longitude \\
    *      +8                    & 4 bytes  & distance \\
    *      +12                   & 4 bytes  & ItemID \\
    *      +16                   & 2 bytes  & Offset \\
    *      +18                   & 2 bytes  & IsItemID or ISLatLon \\
    *      +20                   & 1 byte   & useString \\
    *      +21                   & 1 byte   & nbrSortedHits \\
    *      +22                   & 2 bytes  & categoryType  \\
    *      +24                   & 4 bytes  & matchType \\
    *      +28                   & 4 bytes  & stringPart \\
    *      +32                   & 4 bytes  & sortingType \\
    *      +36                   & 4 bytes  & proximityItems \\
    *      +40                   & 4 bytes  & requestedLanguage \\
    *      +44                   & 4 bytes  & regionsInMatches \\
    *      +48                   & 4 bytes  & innerRadius \\
    *      +52                   & 2 bytes  & startAngle \\
    *      +54                   & 2 bytes  & stopAngle \\
    *      +56                   & 2 bytes  & maxNumberHits \\
    *      +58                   & 1+ bytes & searchString \\
    *   \end{tabular}
    *
    * @param packetID is the ID of the packet.
    * @param reqID is the id of the request sending the packet.
    */
   ProximityPositionRequestPacket( uint16 packetID,
                                   uint16 reqID );
   

   /**
    * Put new proximity data into the packet.
    * @param latitude is the latitude of the position. See Utility for
    *        conversions.
    * @param longitude is the longitude of the position. See Utility for
    *        conversions.
    * @param maxNumberHits is the maximum amount of hits go get back.
    * @param useString is if the searchString should be used to 
    *        limit search hits.
    * @param searchString is the string to search for in the area.
    * @param requestedLanguage The prefered language of serach matches.
    * @param regionsInMatches The search region types or:ed together 
    *                         that should be in the reply matches.
    * @param innerRadius The inner radius. Default 0.
    * @param startAngle The starting angle. Default 0.
    * @param stopAngle The stop angle. Default 360.
    */
   void encodeRequest( PositionRequestPacketContent packetContent,
                       int32 latitude,
                       int32 longitude,
                       uint32 distance,
                       uint32 itemID,
                       uint16 offset,
                       proximitySearchItemsType proximityItems = 
                          PROXIMITY_SEARCH_ITEMS_NUMBERS,
                       uint16 maxNumberHits = 500,
                       bool useString = false,
                       const char* searchString = "",
                       uint16 categoryType = 
                       ( SEARCH_COMPANIES |
                         SEARCH_CATEGORIES ),
                       SearchTypes::StringMatching matchType =
                       SearchTypes::CloseMatch,
                       SearchTypes::StringPart stringPart =
                       SearchTypes::Beginning,
                       SearchTypes::SearchSorting sortingType = 
                       SearchTypes::ConfidenceSort,
                       uint8 nbrSortedHits = MAX_UINT8,
                       uint32 requestedLanguage = StringTable::SWEDISH,
                       uint32 regionsInMatches = 0,
                       uint32 innerRadius = 0,
                       uint16 startAngle = 0,
                       uint16 stopAngle = 360 );


   /**
    * Sets the parameters to the values of the packet.
    */
   void decodeRequest( PositionRequestPacketContent& packetContent,
                       int32& latitude,
                       int32& longitude,
                       uint32& distance,
                       uint32& itemID,
                       uint16& offset,
                       proximitySearchItemsType& proximityItems,
                       uint16& maxNumberHits,
                       bool& useString,
                       char*& searchString,
                       uint16& categoryType,
                       SearchTypes::StringMatching& matchType,
                       SearchTypes::StringPart& stringPart,
                       SearchTypes::SearchSorting& sortingType,
                       uint8& nbrSortedHits,
                       uint32& requestedLanguage,
                       uint32& regionsInMatches,
                       uint32& innerRadius,
                       uint16& startAngle,
                       uint16& stopAngle );
};


class ProximityItemSetRequestPacket;

/**
 *    Reply to a ProximityPositionRequestPacket or 
 *    ProximityItemSetRequestPacket.
 *
 */
class ProximityReplyPacket : public ReplyPacket {
  public:
   /**
    * Answer to a ProximityPositionRequestPacket. \\
    * Encoded as follows: \\
    * \begin{enumerate}
    *    \item setNumberMaps is called with the total number of maps.
    *    \item The mapID and number of items is set for a map and all 
    *          items for the map is added.
    *    \item Prevoius step is repeated for all maps.
    * \end{enumerate}
    * 
    *   After the normal header the reply packet contains:
    *   \begin{tabular}{lll}
    *      Pos                   & Size     & Destription \\ \hline
    *      REPLY_HEADER_SIZE     & 4 bytes  & numberMaps \\
    *      +4                    & 1 bytes  & moreHits \\
    *      for each Map          & 4 bytes  & MapID \\
    *      end for each          & 4 bytes  & nbrItems \\
    *      for each Map*nbrItems & 4 bytes  & ItemID \\
    *   \end{tabular}
    */
   ProximityReplyPacket( ProximityPositionRequestPacket* inPacket);

   
   /**
    * Answer to a ProximityItemSetRequestPacket.\\
    * See other constructor for encoding.
    */
   ProximityReplyPacket( ProximityItemSetRequestPacket* inPacket);

   
   /**
    * Sets the total number of maps in the reply.
    * @param numberMaps is the amount of maps in the packet.
    */
   void setNumberMaps(uint16 numberMaps);


   /**
    * Returns the number of maps in the packet.
    */
   uint16 getNumberMaps();


   /**
    * Sets more hits if there are more hits than in the packet.
    */
   void setMoreHits(uint8 moreHits);


   /**
    * Returns the morehits.
    */
   uint8 getMoreHits();


   /**
    * Set the ID of map with index index.
    * @param index is the index of the map to be set.
    * @param mapID is the ID to set to map index index.
    */
   void setMapID(uint16 index, uint32 mapID);

   
   /**
    * Return the mapID of map with index index
    */
   uint32 getMapID(uint16 index);


   /**
    * Sets the amount of items in map with index index.
    * @param index is the index of the map.
    * @param nbrItems is the amount of items in map with index index.
    * @return the position for the first item for this map. Is only valid 
    *         if the number of items in the maps before this has been 
    *         entered.
    */
   int setNumberItems(uint16 index, uint32 nbrItems);

   
   /**
    * Returns the amount of items for map with index index.
    */
   uint32 getNumberItems(uint16 index);


   /**
    * Add an item to the packet.
    * See setNumberItems for information about position.
    *
    * @return true if the item was added, false if packet was full.
    */
   bool addItem(uint32 itemID,
                int& position);


   /**
    * Returns the position of the first item for map with index index,
    * to be used in getNextItem.
    * @param index is the index of the map to get items from.
    */
   int getFirstItemPosition(uint16 index);


   /**
    * Returns the next item in the packet.
    * @param position is used and updated. Must be set with getFirstItem
    *        first or unpredictiable results are generated.
    * @param strTabIndex is set to the string index, into the stringtable 
    *        for the map.
    */
   uint32 getNextItem(int& position);
};


/**
 *    Packet for proximity search from a set of items.
 *
 */
class  ProximityItemSetRequestPacket : public RequestPacket {
  public:
   /**
    * Packet for asking for hits in a set of items on a set of maps.
    * Is encoded as follows: \\
    * \begin{enumerate}
    *    \item encodeRequest is called with the total number of maps.
    *    \item The mapID and number of items is set for a map and all 
    *          items for the map is added.
    *    \item Prevoius step is repeated for all maps.
    * \end{enumerate}
    *
    *   After the normal header the request packet contains:
    *   \begin{tabular}{lll}
    *      Pos                   & Size     & Destription \\ \hline
    *      REQUEST_HEADER_SIZE   & 4 bytes  & numberMaps \\
    *      +4                    & 1 byte   & useString \\
    *      +5                    & 1 byte   & nbrSortedHits \\
    *      +6                    & 2 bytes  & categoryType  \\
    *      +8                    & 4 bytes  & matchType \\
    *      +12                   & 4 bytes  & stringPart \\
    *      +16                   & 4 bytes  & sortingType \\
    *      +20                   & 4 bytes  & searchStringLength \\
    *      +24                   & 4 bytes  & proximityItems \\
    *      +28                   & 2 bytes  & maximumNumberHits \\
    *      +30                   & 1+ bytes & searchString \\
    *      for each Map          & 4 bytes  & MapID \\
    *      end for each Map      & 4 bytes  & nbrItems \\
    *      for each Map*nbrItems & 4 bytes  & ItemID \\
    *   \end{tabular}
    *
    * @param packetID is the ID of the packet.
    * @param reqID is the id of the request sending the packet.
    */
   ProximityItemSetRequestPacket( uint16 packetID,
                                  uint16 reqID );


   /**
    * Put new proximity data into the packet.
    * @param numberMaps is how many maps.
    * @param maxNumberHits is the maximum amount of hits go get back.
    * @param useString is if the searchString should be used to 
    *        limit search hits.
    * @param searchString is the string to search for in the area.
    *        If useString is false this value is ignored.
    */
   void encodeRequest( uint16 numberMaps,
                       proximitySearchItemsType proximityItems = 
                       PROXIMITY_SEARCH_ITEMS_NUMBERS,
                       uint16 maxNumberHits = 500,
                       bool useString = false,
                       const char* searchString = "",
                       uint16 categoryType = 
                       ( SEARCH_STREETS |
                         SEARCH_COMPANIES |
                         SEARCH_CATEGORIES ),
                       SearchTypes::StringMatching matchType =
                       SearchTypes::CloseMatch,
                       SearchTypes::StringPart stringPart =
                       SearchTypes::Beginning,
                       SearchTypes::SearchSorting sortingType = 
                       SearchTypes::ConfidenceSort,
                       uint8 nbrSortedHits = MAX_UINT8 );


   /**
    * Reads data from the packet.
    */
   void decodeRequest( uint16& numberMaps,
                       proximitySearchItemsType& proximityItems,
                       uint16& maxNumberHits,
                       bool& useString,
                       char*& searchString,
                       uint16& categoryType,
                       SearchTypes::StringMatching& matchType,
                       SearchTypes::StringPart& stringPart,
                       SearchTypes::SearchSorting& sortingType,
                       uint8& nbrSortedHits );
   
   
   
   /**
    * Returns the number of maps in the packet.
    */
   uint16 getNumberMaps();


   /**
    * Set the ID of map with index index.
    * @param index is the index of the map to be set.
    * @param mapID is the ID to set to map index index.
    */
   void setMapID(uint16 index, uint32 mapID);


   /**
    * Return the mapID of map with index index
    */
   uint32 getMapID(uint16 index);

   
   /**
    * Sets the amount of items in map with index index.
    * @param index is the index of the map.
    * @param nbrItems is the amount of items in map with index index.
    * @return the position of the first item for this map. Is only valid 
    *         if the number of items in the maps before this has been 
    *         entered.
    */
   int setNumberItems(uint16 index, uint32 nbrItems);
   
   
   /**
    * Returns the amount of items for map with index index.
    */
   uint32 getNumberItems(uint16 index);


   /**
    * Add an item to the packet.
    * See setNumberItems for information about position.
    * @return true if the item was added, false if packet was full.
    */
   bool addItem(uint32 itemID, int& position);

   
   /**
    * Returns the position of the first item in the packet 
    * for a specific map with index index.
    * @param index is the index of the map to get items from.
    */
   int getFirstItemPosition(uint16 index);


   /**
    * Returns the next item in the packet.
    * @param position is used and updated. 
    * Must be from getFirstItemPosition
    * or unpredictiable results are generated.
    */
   uint32 getNextItem(int& position);
};
#endif
