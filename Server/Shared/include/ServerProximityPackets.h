/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERVER_PROXIMITY_PACKETS_H
#define SERVER_PROXIMITY_PACKETS_H


#define SERVER_PROXIMITY_REPLY_PRIO   DEFAULT_PACKET_PRIO
#define SERVER_PROXIMITY_REQUEST_PRIO DEFAULT_PACKET_PRIO

class UserUser;

#include "config.h"

#include <set>

#include "Types.h"
#include "Packet.h"
#include "SharedProximityPackets.h"
#include "SearchPacket.h"
#include "ItemTypes.h"
#include "TrafficDataTypes.h"
#include "StringTable.h"
#include "LangTypes.h"


class UserRightsMapInfo;
/**
 *    Packet for asking for covered Maps or Items. This packet is used to 
 *    ask for IDs covered by a position and distance from the position or 
 *    asking for IDs covered by a position from an item and a distance 
 *    from the position.
 *
 *    If mapID is MAX_UINT32 the position and distance is checked.
 *    If the request covers more than one map mapsIDs are returned, 
 *    otherwise covered items in the map are returned. Otherwise if 
 *    mapID is not MAX_UINT32, covered items in the map are returned.
 *
 *    After the normal header the request packet contains:
 *    \begin{tabular}{lll}
 *      Pos                   & Size     & Destription \\ \hline
 *      REQUEST_HEADER_SIZE   & 4 bytes  & latitude \\
 *      +4                    & 4 bytes  & longitude \\
 *      +8                    & 4 bytes  & distance \\
 *      +12                   & 4 bytes  & ItemID \\
 *      +16                   & 2 bytes  & Offset \\
 *      +18                   & 2 bytes  & IsItemID or IsLatLon \\
 *      +20                   & 4 bytes  & Inner radius \\
 *      +24                   & 4 bytes  & Start angle \\
 *      +28                   & 4 bytes  & Stop angle \\
 *      +32                   & 4 bytes  & Nbr item types \\
 *      +36                   & 4 bytes * nbr & item types \\
 *   \end{tabular}
 *
 */
class CoveredIDsRequestPacket : public RequestPacket {
   public:
      /**
       *    Create a new packet.
       *    @param packetID   The ID of the packet.
       *    @param reqID      The ID of the request sending the packet.
       *    @param mapID      The map to extract items from.
       *    @param itemTypes  The item types to return. Empty set means all
       *                      types.
       */
      CoveredIDsRequestPacket( const UserUser* user,
                               uint16 packetID,
                               uint16 reqID,
                               uint32 mapID,
                               const set<ItemTypes::itemType>& itemTypes =
                               set<ItemTypes::itemType>());

      /**
       *    Returns the number of requested itemtypes.
       *    @param types Where to put the requested item types.
       *    @param rights User rights for the user that sent the packet.
       *    @return The number of requested item types.
       */
      int getItemTypesAndRights(set<ItemTypes::itemType>& types,
                                UserRightsMapInfo& rights) const;
      
      /**
       *    Set the latitude.
       *    @param   lat   The new value of the latitude part of the 
       *                   centre of the circle, sector or arc.
       */
      void setLat(int32 lat);

      /**
       *    Get the latitude part of the centre coordinate.
       *    @return  The latitude for the centre coordinate..
       */
      int32 getLat() const;

      /**
       *    Set the longitude.
       *    @param   lon   The new value of the longitude part of the
       *                   centre of the circle, sector or arc.
       */
      void setLon(int32 lon);

      /**
       *    Get the latitude part of the centre coordinate.
       *    @return  The latitude for the centre coordinate..
       */
      int32 getLon() const;

      /**
       *    Does this packet contain ItemID or coordinate as center point?
       *    @return  True if the packet contains an ItemID as position, 
       *             false otherwise.
       */
      bool isItemID() const;

      /**
       *    Set that the packet contains a valid ItemID.
       */
      void setItemIDAsValid();   

      /**
       *    Does this packet contain coordinae as center point?
       *    @return True if the packet contains lat,lon as position.
       */
      bool isLatLon() const;

      /**
       *    Set that the packet contains valid lat, lon.
       */
      void setLatLonAsValid();

      /**
       *    Set the ItemID, describing the centre point.
       *    @param   itemID   The ID of the item in the middle of the
       *                      circle, sector or arc.
       */
      void setItemID(uint32 itemID);

      /**
       *    Get the ItemID of the item in the centre.
       *    @return  The ID of the item in the centre.
       */ 
      uint32 getItemID() const;

      /**
       *    Set the offset of the centre point.
       *    @param   offset   The offset at the item describing the 
       *                      centre point. 
       */
      void setOffset(uint16 offset);

      /**
       *    Get the offset at item with given ID.
       *    @return The offset on the item.
       */
      uint16 getOffset() const;

      /**
       *    @name Radius
       *    Methods to get and set the radiuses of the area.
       */
      //@{
         /**
          *    Set the outer radius of the specified area (in meters).
          *    @param   r  The new value of the outer radius.
          */
         void setOuterRadius(uint32 r);

         /**
          *    Get the outer radius.
          *    @return  The maximum distance from the centre point (in 
          *             meters).
          */
         uint32 getOuterRadius() const;

         /**
          *    Set the inner radius (in meters).
          *    @param   r  The new value of the inner radius.
          */
         void setInnerRadius(uint32 r);

         /**
          *    Get the value of the inner radius (in meters).
          *    @return  The minimum distance from the centre point.
          */
         uint32 getInnerRadius() const;
      //@}

      /**
       *    @name Angles
       *    Methods to get and set the values of the angles.
       */
      //@{
         /**
          *    Set the value of the start-angle (in degrees from the 
          *    north-direction).
          *    @param   angel The value of the start-angle.
          */
         void setStartAngle(uint32 angle);

         /**
          *    Get the value of the start-angle (in degrees from the 
          *    north-direction).
          *    @return The value of the start-angle.
          */
         uint32 getStartAngle() const;

         /**
          *    Set the value of the stop-angle (in degrees from the 
          *    north-direction).
          *    @param   angel The value of the stop-angle.
          */
         void setStopAngle(uint32 angle);

         /**
          *    Get the value of the stop-angle (in degrees from the 
          *    north-direction).
          *    @return The value of the stop-angle.
          */
         uint32 getStopAngle() const;
      //@}
};


/**
 *    Reply to a CoveredIDsRequestPacket. After the normal header 
 *    this request packet contains:
 *    \begin{tabular}{lll}
 *       {\bf Pos}            & {\bf Size}& {\bf Destription} \\ \hline
 *       REPLY_HEADER_SIZE    & 4 bytes   & mapID, is MAX_UINT32 if 
 *                                          packet contains MapIDs   \\
 *       +4                   & 4 bytes  &  number IDs
 *                                         (stored as length of packet)\\
 *       for each ID          & 4 bytes  & ID \\
 *   \end{tabular}
 *
 */
class CoveredIDsReplyPacket : public ReplyPacket {
   public:
      /**
       *    Reply with the IDs in the covered area.
       *    @param   inPacket The packet that this is a reply to.
       */
      CoveredIDsReplyPacket(const CoveredIDsRequestPacket* inPacket,
                            int packetSize = MAX_PACKET_SIZE);

      /**
       *    Calculates the needed packet size for the supplied number
       *    of nodes.
       *    @param nbrNodes The number of nodes to add.
       *    @return The required packet size for the number of nodes
       *            to fit into the packet.
       */
      static int calcPacketSize(int nbrNodes);
      
      /**
       *    Set the mapID of this reply. If set to MAX_UINT32 the content
       *    is beleaved to be MapIDs otherwise ItemIDs.
       *    @param   mapID The ID of the map where the items are located.
       */
      void setMapID(uint32 mapID);
      
      /**
       *    Tells if the IDs are MapIDs or ItemIDs.
       *    @return  True if this packet contains IDs of maps, false 
       *             otherwise.
       */
      bool isMapIDs() const;

      /**
       *    Get the ID of the map where the items are located.
       *    @return  ID of the map.
       */
      uint32 getMapID() const;

      /**
       *    The amount IDs in the reply. Use the method #isMapIDs()# to find
       *    out if it is IDs of maps or items.
       *    @return  The number of IDs in this packet.
       */
      uint32 getNumberIDs() const;

      /**
       *    Add an ID.
       *    @param   ID  The new ID, that should be added to the packet.
       *    @param  type The type of the item.
       *    @return True if the item was added, false if packet was full.
       */
      bool addID(uint32 ID, ItemTypes::itemType type);

      /**
       *    Returns an ID.
       *    @param   index The cardinal number of the ID to return. Valid
       *                   values are 0 <= index < getNumberIDs().
       *    @return  ID number index. MAX_UINT32 is returned upon error.
       */
      uint32 getID(uint32 index) const;

      /**
       *    Returns the type of the item at position idx.
       */
      ItemTypes::itemType getType(uint32 idx) const;

   private:
      /**
       *    Set the number of items to a new value.
       *    @param   newNbrIDs The new value of the number of items.
       */
      void setNumberIDs(uint32 newNbrIDs);

      /**
       *    @name Constants
       *    Constants that stores the positions inside this packet.
       */
      //@{
         /**
          *    The position of the map ID.
          */
         static const uint32 m_mapIDPos;

         /**
          *    The position of the number of items field.
          */
         static const uint32 m_nbrIDsPos;

         /**
          *    The position of the first ID.
          */
         static const uint32 m_firstIDPos;
      //@}
}; // CoveredIDsReplyPacket 


/**
 *    Packet for selecting Items, matching a specific search, from a set.
 *    After the normal header the request packet contains:
 *    \begin{tabular}{lll}
 *       {\bf Name of field}   & {\bf Field size}& {\bf Position} \\ \hline
 *                             &                 & getHeaderEndPosition() \\
 *       nbr items             &  4 bytes        & +4             \\
 *       useString             &  1 byte         & +1             \\
 *       nbrSortedHits         &  1 byte         & +1             \\
 *       categoryType          &  2 bytes        & +2             \\
 *       matchType             &  4 bytes        & +4             \\
 *       stringPart            &  4 bytes        & +4             \\
 *       length of searchString&  4 bytes        & +4             \\
 *       proximityItems        &  4 bytes        & +4             \\
 *       requestedLanguage     &  4 bytes        & +4             \\
 *       regionsInMatches      &  4 bytes        & +4             \\
 *       maxNbrHits            &  2 bytes        & +2             \\
 *       searchString          &  strlen(s)+1    & +strlen(s)+1
 *    \end{tabular}
 *
 */
class ProximitySearchRequestPacket : public OldSearchRequestPacket {
   public:
      /**
       *    Packet for finding relevant matches from a set of Items.
       *
       *    @param   packetID ID of this packet.
       *    @param   reqID    ID of the request that created this packet, and
       *                      that will receive the answer.
       *    @param   mapID    ID of the map where the items in this packet are
       *                      located.
       */
      ProximitySearchRequestPacket(uint16 packetID,
                                   uint16 reqID ,
                                   uint32 mapID );
      /**
       *    Put information into the packet.
       *    @param proximityItems   The type of search.
       *    @param maximumNbrHits   The maximum number of hits that will
       *                            be returned.
       *    @param useString        True if the searchString should be
       *                            used to limit search hits.
       *    @see VanillaSearchRequestPacket
       */
      void encodeRequest( uint8 dbMask,
                          proximitySearchItemsType proximityItems = 
                             PROXIMITY_SEARCH_ITEMS_NUMBERS,
                          uint16 maximumNumberHits = 500,
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
                          uint8 nbrSortedHits = MAX_UINT8,
                          LangTypes::language_t requestedLanguage =
                          LangTypes::swedish,
                          uint32 regionsInMatches = 0 );

      /**
       *   Same as above, but without default values and with
       *   StringTable::languageCode. The languageCode will be
       *   translated into the right kind of code. (Which is
       *   really is language_t).
       */
      void encodeRequest( uint8 dbMask,
                          proximitySearchItemsType proximityItems,
                          uint16 maximumNumberHits,
                          bool useString,
                          const char* searchString,
                          uint16 categoryType,
                          SearchTypes::StringMatching matchType,
                          SearchTypes::StringPart stringPart,
                          SearchTypes::SearchSorting sortingType,
                          uint8 nbrSortedHits,
                          StringTable::languageCode requestedLanguage,
                          uint32 regionsInMatches) {
         encodeRequest(dbMask, proximityItems, maximumNumberHits,
                       useString, searchString, categoryType,
                       matchType, stringPart,
                       sortingType, nbrSortedHits,
                       ItemTypes::getLanguageCodeAsLanguageType(
                          requestedLanguage),
                       regionsInMatches);
      }

      /**
       *    Return the data in the packet except for the itemsIDs.
       */
      void decodeRequest( uint8& dbMask,
                          proximitySearchItemsType& proximityItems,
                          uint32& nbrItems,
                          uint16& maximumNumberHits,
                          bool& useString,
                          char*& searchString,
                          uint16& categoryType,
                          SearchTypes::StringMatching& matchType,
                          SearchTypes::StringPart& stringPart,
                          SearchTypes::SearchSorting& sortingType,
                          uint8& nbrSortedHits,
                          uint32& requestedLanguage,
                          uint32& regionsInMatches );
      
      /**
       *    Add an item to the packet. The field that stores the number of 
       *    items is increased by one. The last parameter must be initialized
       *    with a call to getFirstItemPosition().
       *    @see getItem() Example of usage of getFirstItemPosition().
       *    @return true if the item was added, false if packet was full.
       */
      bool addItem(uint32 itemID, int& position);
      
      /**
       *    Returns the position of the first item. Used to initalize the 
       *    parameter in getItem and addItem.
       *    @return  The position of the first item in this packet.
       */
      int getFirstItemPosition();

      /**
       *    Get the next item. The parameter must be initialized with a call
       *    to #getFirstItemPosition()#. An example of usage is:
       *    \begin{verbatim}
            int pos = p->getFirstItemPosition();
            uint32 n = p->getNbrItems();
            for (uint32 i=0; i<n; << i++)
               cerr << i << " itemID = " << getItem(pos) << endl;
            \end{verbatim}
       *    @see getNbrItems  Observe the comment about this expensive call.
       *    @return  The ID of the next item.
       */
      uint32 getItem(int& position);

      /**
       *    Get the amount of items in the packet. {\it {\bf NB!} This call
       *    is quite expensive (since the position of this attribute not is
       *    constant but must be calculated each time).}
       *    @return  The number of items that is added to this packet.
       */
      uint32 getNbrItems();
};


/**
 *    Reply to a ProximitySearchRequestPacket.
 *    After the normal header the request packet contains:
 *    \begin{tabular}{lll}
 *       {\bf Pos}             & {\bf Size}  & {\bf Destription}  \\ \hline
 *       REPLY_HEADER_SIZE     & 4 bytes     & numberItems        \\
 *       +4                    & 1 byte      & moreHits           \\
 *       +5                    & 3 bytes     & {\it unused}       \\
 *       for each Item         & 4 bytes     & ItemID             \\
 *    \end{tabular}
 *
 */
class ProximitySearchReplyPacket : public SearchReplyPacket {
   public:
      /**
       *    Packet holding answer to a ProximitySearchRequestPacket.
       *    @param   inPacket The packet that this is a reply to.
       */
      ProximitySearchReplyPacket( ProximitySearchRequestPacket* inPacket );

      /**
       *    Get the ID of the map for the items.
       *    @return  ID of the map where the items in this packet are 
       *             located.
       */
      uint32 getMapID();

      /**
       *    Set the ID of the map for the items.
       *    @param   newID The new ID of the map where the items in this 
       *                   packet are located.
       */
      void setMapID(uint32 newID);
      
      /**
       *    Return the number of items.
       *    @return  The number of items included in this reply packet.
       */
      uint32 getNumberItems();

      /**
       *    Sets more hits if there are more hits than fitted into
       *    this packet.
       *    @param   moreHits 
       */
      void setMoreHits(uint8 moreHits);

      /**
       *    Returns the value of the morehits-member.
       */
      uint8 getMoreHits();

      /**
       *    Add one item to the packet. The second parameter should be
       *    initialized ba a call to the getFirstItemPostion()-method.
       *    @param   itemID   ID of the item that should be added to 
       *                      this packet.
       *    @param   position In-out parameter, that tells the position
       *                      where the item should be added. Updated 
       *                      inside this method.
       *    @return True if the item was added, false if packet was full.
       */
      bool addItem(uint32 itemID, int& position); 

      /**
       *    Return the position of the first item. This method should be
       *    used to initialize the parameter to #addItem()# and #getItem()#.
       *    @return  The position of the first item in this packet.
       */
      int getFirstItemPosition();

      /**
       *    Get the next item in this packet. The parameter should be 
       *    initialized ba a call to getFirstItemPosition.
       *    @param   position In-out parameter, that tells the position
       *                      to read from. Updated inside this method.
       *    @return The next item in this packet.
       */
      uint32 getItem(int& position);

   private:
      /**
       *    @name Constants
       *    Constants that stores the positions inside this packet.
       */
      //@{
         /**
          *    Position of the ID of the map.
          */
         static const uint32 m_mapIDPos;

         /**
          *    Position of the number of items field.
          */
         static const uint32 m_nbrItemsPos;

         /**
          *    Position of the more items field.
          */
         static const uint32 m_moreItemsPos;

         /**
          *    Position of the first item ID in this packet.
          */
         static const uint32 m_firstItemPos;
      //@}
};


/**
 *    Packet for asking for roads to place traffic on.
 *    Differs from coverdIDs and coord requests as we always look for
 *    SSIs suitable for traffic.
 *  
 *    After the normal header the request packet contains:
 *    \begin{tabular}{lll}
 *      Pos                   & Size     & Destription \\ \hline
 *      REQUEST_HEADER_SIZE   & 4 bytes  & latitude \\
 *      +4                    & 4 bytes  & longitude \\
 *      +8                    & 4 bytes  & maxRadius \\
 *      +16                   & 2 bytes  & angle \\
 *      +18                   & 1 byte   & nbrOfHits \\
 *      +20                   & 1 byte   & maxRoadClass \\
 *      +24                   & ? bytes  & roadName \\
 *   \end{tabular}
 *
 */
class TrafficPointRequestPacket  : public RequestPacket {
   public:
   TrafficPointRequestPacket( const UserUser* user,
                              uint16 packetID, uint16 reqID,
                              uint32 mapID);
   
   /**
    *    Returns the User rights for.
    *    @param rights User rights for the user that sent the packet.
    *    @return The number of requested item types.
    */
   bool getUserRights(UserRightsMapInfo& rights) const;

   void addValues(const MC2Coordinate& center,
                  uint32 maxRadius,
                  int nbrOfHits,
                  uint16 angle,
                  TrafficDataTypes::direction direction,
                  uint8 maxRoadClass);
   
   void addRoadName(const MC2String roadName);

   uint32 getLat() const;

   uint32 getLon() const;

   uint32 getMaxRadius() const;

   int getNbrOfHits() const;

   uint16 getAngle() const;

   TrafficDataTypes::direction getTrafficDirection() const;

   uint8 getMaxRoadClass() const;
   
};

/**
 *    Reply to a .TrafficPointRequestPacket
 *    After the normal header the request packet contains:
 *    \begin{tabular}{lll}
 *       {\bf Pos}             & {\bf Size}  & {\bf Destription}  \\ \hline
 *    \end{tabular}
 *
 */
class TrafficPointReplyPacket : public ReplyPacket {
   public:
    TrafficPointReplyPacket(const TrafficPointRequestPacket* inPacket,
                            int packetSize = MAX_PACKET_SIZE);


      /**
       *    Set the mapID of this reply. If set to MAX_UINT32 the content
       *    is beleaved to be MapIDs otherwise ItemIDs.
       *    @param   mapID The ID of the map where the items are located.
       */
      void setMapID(uint32 mapID);
      
      /**
       *    Tells if the IDs are MapIDs or ItemIDs.
       *    @return  True if this packet contains IDs of maps, false 
       *             otherwise.
       */
      bool isMapIDs() const;

      /**
       *    Get the ID of the map where the items are located.
       *    @return  ID of the map.
       */
      uint32 getMapID() const;

      /**
       *    The amount IDs in the reply. Use the method #isMapIDs()# to find
       *    out if it is IDs of maps or items.
       *    @return  The number of IDs in this packet.
       */
      uint32 getNumberIDs() const;

      /**
       *    Add an ID.
       *    @param   ID  The new ID, that should be added to the packet.
       *    @param  type The type of the item.
       *    @return True if the item was added, false if packet was full.
       */
      bool addID(uint32 ID);

      /**
       *    Returns an ID.
       *    @param   index The cardinal number of the ID to return. Valid
       *                   values are 0 <= index < getNumberIDs().
       *    @return  ID number index. MAX_UINT32 is returned upon error.
       */
      uint32 getID(uint32 index) const;

   private:
      /**
       *    Set the number of items to a new value.
       *    @param   newNbrIDs The new value of the number of items.
       */
      void setNumberIDs(uint32 newNbrIDs);

      /**
       *    @name Constants
       *    Constants that stores the positions inside this packet.
       */
      //@{
         /**
          *    The position of the map ID.
          */
         static const uint32 m_mapIDPos;

         /**
          *    The position of the number of items field.
          */
         static const uint32 m_nbrIDsPos;

         /**
          *    The position of the first ID.
          */
         static const uint32 m_firstIDPos;
      //@}
   
};

// ========================================================================
//                                     Implementation of inline functions =

#endif // SERVER_PROXIMITY_PACKETS_H 

