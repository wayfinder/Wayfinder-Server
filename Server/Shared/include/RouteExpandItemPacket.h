/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTEEXPANDITEMPACKET_H
#define ROUTEEXPANDITEMPACKET_H

#include "Packet.h"
#include "Types.h"
#include "config.h"
#include "ItemTypes.h"

#define ROUTEEXPANDITEM_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define ROUTEEXPANDITEM_REPLY_PRIO   DEFAULT_PACKET_PRIO

/**
 *    Get the street segment item IDs, offsets and coordinates of an 
 *    item. For example if a company with multiple entrances or a street 
 *    without house numbers is sent, it is expanded to several positions.
 *    If something else is sent, its position is returned.
 * 
 *    Get coordinates of one item. This packet should be used when 
 *    requesting the coordinates of an item with known ID. It is also
 *    possible to give an offset at the item to specify the coordinate
 *    more accurate.
 *  
 *    After the normal header the request packet contains:
 *    @packetdesc
 *    @row 0  @sep 4 bytes @sep User data              @endrow
 *    @row 4  @sep 4 bytes @sep Nbr of items in packet @endrow
 *    @row 8  @sep 4 bytes @sep Preferred language     @endrow
 *    @row 12+x*8 @sep 8 bytes @sep Item id and offset @endrow
 *    @endpacketdesc
 *
 *    Based on CoordinateOnItemPacket.
 */
class RouteExpandItemRequestPacket : public RequestPacket
{
  public:
   /** 
    *   Full version of the constructor.
    *   @param   packetID The packet ID.
    *   @param   reqID    The request ID.
    *   @param   userData Data to be copied from the request to the
    *                     reply.
    */
   RouteExpandItemRequestPacket(uint32 packetID,
                                uint32 reqID,
                                uint32 userData = MAX_UINT32);

   /**
    *   Add one item to be expanded to the packet.
    *   @param   itemID  The item ID.
    *   @param   offset  The offset from node 0.
    *                    The offset will be ignored if this is a
    *                    company or whatever with multiple entries.
    */
   void add(uint32 itemID, uint16 offset = 0x7fff);
    
   /**
    *    Get one of the records in this packet.
    *    @param index   The index of the itemID. Valid values are
    *                   #0 <= index < getNbrItems()#.
    *    @param itemID  Outparameter that is set to the ID of 
    *                   item number index.
    *    @param offset  Outparameter that is set to the offset
    *                   from node 0 for item number index.
    *                   The offset will be ignored if this is a
    *                   company or whatever with multiple entries.
    */
   void getData( int32 index, uint32& itemID, uint16& offset ) const;

   /**
    *    Get the number of items in the packet.
    *    @return  The number of items in this packet.
    */
   inline uint32 getNbrItems() const;

   /**
    *    @return The user defined data for the packet.
    */
   inline uint32 getUserData() const;
   
  private:
   /**
    *   Set the number of items in this request
    *   @param num  The number of items in this request.
    */
   inline void setNbrItems( uint32 num );

   /** The position of the user data in the packet */
   static const int USER_DATA_POS = REQUEST_HEADER_SIZE;
   /** The position of the nbr of items in the packet */
   static const int NBR_ITEMS_POS = USER_DATA_POS + 4;
   /** The position of the preferred language. Unused? */
   static const int LANGUAGE_POS  = NBR_ITEMS_POS + 4;
   /** The position where the items start */
   static const int ITEMS_START_POS = LANGUAGE_POS + 4;
   
   
   /** The size of one item in the packet */
   static const int ITEM_SIZE = 8; /* ItemID + offset */
   
};

/**
 *    The reply of an RouteExpandItemRequestPacket. After the normal 
 *    header this packet contains (subType = ROUTEEXPANDITEM_REPLY
 *    and 0 below is REPLY_HEADER_SIZE):
 *    @packetdesc
 *    \begin{tabular}{lll}
 *       {\bf Pos} & {\bf Size} & {\bf Description} \\ \hline
 *       $X$       & 4 bytes    & User data \\
 *       $X+4$     & 4 bytes    & Map id \\
 *       $X+8$     & 4 bytes    & Nbr of items in result \\
 *       for each  & 2 byte     & The number of positions associated
 *                                with the current item.\\
 *                 & 1 byte     & Parents ItemType type. \\
 *                 & 1 byte     & Padding.\\
 *                 & 4 bytes    & ID of expanded item.\\
 *       for each 
 *       position  & 4 bytes    & The street segment itemID.\\
 *                 & 2 bytes    & The offset.\\
 *                 & 2 bytes    & Padding.\\
 *                 & 4 bytes    & The latitude part of the coordinate at
 *                                requested offset.\\
 *       end each  & 4 bytes    & The longitude part of the coordinate at
 *                                requested offset.\\
 *    \end{tabular}
 *      
 */
class RouteExpandItemReplyPacket : public ReplyPacket
{
   public:
      /**
       *   Creates a RouteExpandItemReplyPacket as an reply to a given
       *   request.
       *   @param   p  The corresponding RouteExpandItemRequestPacket.
       */
      RouteExpandItemReplyPacket(const RouteExpandItemRequestPacket* p);

      /**
       *   Add one item to this packet. Allocates bigger buffer if needed.
       *   @param   nbrPos     The number of positions associated with\\
       *                       the current item.
       *   @param expandedID   The ID of the item that was expanded.
       *   @param type         The type of the parents to the 
       *                       streetsegmentitems.
       *   @param   ssitemID   An array of streetsegmentitemIDs.
       *   @param   offset     An array of offsets.
       *   @param   lat        An array of latitudes
       *                       on item with itemID at offset 
       *                       given in the request packet.
       *   @param   lon        An array of longitudes
       *                       on item with itemID at offset 
       *                       given in the request packet.
       *   @param   parentItemID The parents itemIDs for the 
       *            streetsegmentitems in ssItemID.
       */
      void addItem( uint16 nbrPos,
                    uint32 expandedID,
                    ItemTypes::itemType parentType,
                    const uint32* ssItemID,
                    const uint16* offset,
                    const int32* lat,
                    const int32* lon,
                    const uint32* parentItemID);

      /**
       *   Get the number of items in this reply packet.
       *   @return  The number of items.
       */
      inline uint32 getNbrItems() const;

      /**
       *   Init getting items.
       *   @return The position to use the first time getItem is used.
       */
      inline int positionInit();

      /**
       *    Gets the next item.
       *    @param position   The position (in the packet), will get updated.
       *    @param nbrPos     The number of positions (on map) of this item.
       *    @param expandedItemID The ID of the expanded item.
       *   @param type         The type of the parents to the 
       *                       streetsegmentitems.
       *    @param ssItemIDs  An array of street segment item ids.
       *    @param offsets    An array of offsets.
       *    @param lats       An array of latitudes.
       *    @param lons       An array of longitudes.
       *   @param   parentItemID The parents itemIDs for the 
       *            streetsegmentitems in ssItemID.
       *    {\it {\bf NB!} Params ssItemIDs, offsets,
       *    lats, lons and parentItemIDs are allocated 
       *    (using new) in here, but the caller has to delete them!}
       */
      void getItem(int& position,
                   uint16& nbrPos,
                   uint32& expandedItemID,
                   ItemTypes::itemType& parentType,
                   uint32*& ssItemIDs,
                   uint16*& offsets,
                   int32*& lats,
                   int32*& lons,
                   uint32*& parentItemIDs);

      /**
       *    Returns the user defined data for the packet.
       *    Is copied from the request to the reply.
       */
      inline uint32 getUserData() const;

      /**
       *    Returns the map id of the request.
       */
      inline uint32 getMapID() const;
      
  private:
   /**
    *   Set the number of items in this packet to a new value.
    *   @param   num   The new number of items in this packet.
    */
   inline void setNbrItems( uint32 num );

   /**
    *   Sets the map id of the packet.
    */ 
   inline void setMapID( uint32 mapID );

   /**
    *   Sets the user data of the packet.
    */
   inline void setUserData(uint32 userData);

   /** Position of user data */
   static const int USER_DATA_POS   = REQUEST_HEADER_SIZE;
   /** Position of map ID */
   static const int MAP_ID_POS      = USER_DATA_POS + 4;
   /** Position of number of items in result */
   static const int NBR_ITEMS_POS   = MAP_ID_POS + 4;
   /** Position where the items start */
   static const int ITEMS_START_POS = NBR_ITEMS_POS + 4;


};  

// ========================================================================
//                                       Implementation of inline methods =

// - - - - - - - - - - - - - - - - - - - - - - RouteExpandItemRequestPacket
inline uint32 
RouteExpandItemRequestPacket::getNbrItems() const
{
   return readLong(NBR_ITEMS_POS);
}

inline void 
RouteExpandItemRequestPacket::setNbrItems( uint32 num )
{
   writeLong(NBR_ITEMS_POS, num);  
}

inline uint32
RouteExpandItemRequestPacket::getUserData() const
{
   return readLong(USER_DATA_POS);
}

// - - - - - - - - - - - - - - - - - - - - - - - RouteExpandItemReplyPacket
inline int 
RouteExpandItemReplyPacket::positionInit()
{
   return ITEMS_START_POS;
}

inline uint32 
RouteExpandItemReplyPacket::getNbrItems() const
{
   return readLong(NBR_ITEMS_POS);
}

inline void 
RouteExpandItemReplyPacket::setNbrItems( uint32 num )
{
   writeLong(NBR_ITEMS_POS, num);
}

inline void
RouteExpandItemReplyPacket::setUserData( uint32 userData )
{
   writeLong(USER_DATA_POS, userData);
}

inline uint32
RouteExpandItemReplyPacket::getUserData() const
{
   return readLong(USER_DATA_POS);
}

inline void
RouteExpandItemReplyPacket::setMapID( uint32 mapID )
{
   writeLong(MAP_ID_POS, mapID);
}

inline uint32
RouteExpandItemReplyPacket::getMapID() const
{
   return readLong(MAP_ID_POS);
}


#endif
