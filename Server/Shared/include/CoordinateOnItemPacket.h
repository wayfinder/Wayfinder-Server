/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COORDINATEONITEMPACKET_H
#define COORDINATEONITEMPACKET_H

#include "Packet.h"
#include "Types.h"
#include "config.h"

#define COORDINATEONITEM_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define COORDINATEONITEM_REPLY_PRIO   DEFAULT_PACKET_PRIO

#define COORDINATEONITEM_REQUEST_HEADER_SIZE 8
#define COORDINATEONITEM_REPLY_HEADER_SIZE   4

class MC2BoundingBox;

/**
 *    Get coordinates of one item. This packet should be used when 
 *    requesting the coordinates of an item with known ID. It is also
 *    possible to give an offset at the item to specify the coordinate
 *    more accurate.
 *  
 *    After the normal header the request packet contains:
 *    @packetdesc
 *       @row 20       @sep 4 byte     @sep Nbr of items in packet @endrow
 *       @row 24       @sep 4 byte     @sep Flags   @endrow
 *       @row for each @sep 4 byte     @sep item ID @endrow
 *       @row          @sep 2 byte     @sep offset  @endrow
 *    @endpacketdesc
 *
 */
class CoordinateOnItemRequestPacket : public RequestPacket 
{
public:
   
   /** 
    *   Constructor.
    *   @param   packetID The packet ID.
    *   @param   reqID    The request ID.
    *   @param   bboxWanted True if bounding boxes
    *                       should be sent with the reply.
    *                       It is somewhat processor consuming to
    *                       calculate the bounding box, so don't
    *                       use it if you don't have to.
    */
   CoordinateOnItemRequestPacket(uint32 packetID   = 0,
                                 uint32 reqID      = 0,
                                 bool bboxWanted = false,
                                 bool allCoordinatesWanted = false);

   /**
    *   Add one item to the packet.
    *   @param   itemID  The item ID.
    *   @param   offset  The offset from node 0. If offset == MAX_UINT32
    *                    then the center will be used for closed polygons.
    */
   void add(uint32 itemID, uint32 offset = 0x7fff);
   
   /**
    *    Get one of the records in this packet.
    *    @param index   The index of the itemID. Valid values are
    *                   #0 <= index < getNbrItems()#.
    *    @param itemID  Outparameter that is set to the ID of 
    *                   item number index.
    *    @param offset  Outparameter that is set to the offset
    *                   from node 0 for item number index.
    */
   void getData( int32 index, uint32& itemID, uint32& offset ) const;
   
   /**
    *    Get the number of items in the packet.
    *    @return  The number of items in this packet.
    */
   inline uint32 getNbrItems() const;
   
   /**
    *    Return true if bounding boxes should be sent
    *    in the reply. Otherwise non-valid bounding boxes
    *    are sent.
    *    @return True if bounding boxes should be calculated.
    */
   inline bool getBBoxWanted() const;

   inline bool getAllCoordinatesWanted() const;
   
private:
   
   /**
    *    Sets or clears the bounding box wanted-bit.
    */
   inline void setBBoxWanted(bool wanted);

   inline void setAllCoordinatesWanted(bool allCoordinates);
   
   /**
    *   Set the number of items in this request
    *   @param num  The number of items in this request.
    */
   inline void setNbrItems( uint32 num );
   
   /** The position of the number of items */
   static const int NBR_ITEMS_POS = REQUEST_HEADER_SIZE;
   
   /** The position of the flags */
   static const int FLAGS_POS     = NBR_ITEMS_POS + 4;

   /** The bit position of the bounding box wanted bit */
   static const int BBOX_WANTED_BIT_POS = 0;

   static const int ALL_COORDS_WANTED_BIT_POS = 1;
};

/**
 *    The reply of an CoordinateOnItemRequestPacket. After the normal 
 *    header this packet contains (subType = COORDINATEONITEM_REPLY):
 *    @packetdesc
 *       @row 20        @sep 4 bytes    @sep Nbr of items in result @endrow
 *       @row for each  @sep 4 bytes    @sep The ID of the Item @endrow
 *       @row           @sep 4 bytes    
 *                      @sep The latitude part of the coordinate at 
 *                           requested offset.                   @endrow
 *       @row           @sep 4 bytes    
 *                      @sep The longitude part of the coordinate at 
 *                           requested offset.                   @endrow
 *       @row           @sep 4 bytes @sep The maxlat of the bbox. @endrow
 *       @row           @sep 4 bytes @sep The minlon of the bbox. @endrow
 *       @row           @sep 4 bytes @sep The minlat of the bbox. @endrow
 *       @row end each  @sep 4 bytes @sep The maxlon of the bbox. @endrow
 *
 *    @endpacketdesc
 *      
 */
class CoordinateOnItemReplyPacket : public ReplyPacket
{
   public:
      /**
        *   Creates a CoordinateOnItemReplyPacket as an reply to a given
        *   request.
        *   @param   p        The corresponding CoordinateOnItemRequestPacket.
        *   @param   nbrItems The number of items in the reply.
        */
      CoordinateOnItemReplyPacket(const CoordinateOnItemRequestPacket* p,
                                  int nbrItems);

      /**
        *   Add one coordinate to this packet.
        *   @param   itemID   The ID of the item.
        *   @param   lat      The latitude on item with itemID at offset 
        *                     given in the request packet.
        *   @param   lon      The longitude on item with itemID at offset 
        *                     given in the request packet.
        *   @param   bbox     The boundingbox for the item.
        */
      void add( uint32 itemID, int32 lat, int32 lon,
                const MC2BoundingBox* bbox );

      /**
        *   Get the number of items in this reply packet.
        *   @return  The number of coordinates.
        */
      inline uint32 getNbrItems() const;

      /**
        *   Get the coordinate for item nuber index.
        *
        *   @param index   The index of the lat, long in the packet.
        *                  Valid values are #0 <= index < getNbrItems()#.
        *   @param itemID  Outparameter that is set to the ID of item 
        *                  number index.
        *   @param lat     Outparameter that is set to the latitude for 
        *                  item number index.
        *   @param lon     Outparameter that is set to the longitude for 
        *                  item number index.
        */
      void getLatLong(int32 index,
                      uint32& itemID,
                      int32& lat,
                      int32& lon) const;

      /**
        *   Get the coordinate for item nuber index.
        *
        *   @param index   The index of the lat, long in the packet.
        *                  Valid values are #0 <= index < getNbrItems()#.
        *   @param itemID  Outparameter that is set to the ID of item 
        *                  number index.
        *   @param lat     Outparameter that is set to the latitude for 
        *                  item number index.
        *   @param lon     Outparameter that is set to the longitude for 
        *                  item number index.
        *   @param bbox    The bounding box of the Item. Will be filled
        *                  with boundingbox data if possible.
        */
      void getLatLongAndBB(int32 index,
                           uint32& itemID,
                           int32& lat,
                           int32& lon,
                           MC2BoundingBox& bbox) const;

      
      
   private:
      /**
        *   Set the number of items in this packet to a new value.
        *   @param   num   The new number of items in this packet.
        */
      inline void setNbrItems( uint32 num );

      /**
       *    The number of bytes per item.
       *    4 bytes itemid, 4 bytes lat, 4 bytes lon, 16 byte BBox
       */
      static const int BYTES_PER_ITEM = 28;
};  

// ========================================================================
//                                       Implementation of inline methods =

//---------------
// Request
//---------------

inline uint32 
CoordinateOnItemRequestPacket::getNbrItems() const
{
   return readLong(NBR_ITEMS_POS);
}

inline void 
CoordinateOnItemRequestPacket::setNbrItems( uint32 num )
{
   writeLong(NBR_ITEMS_POS, num);  
}

inline void
CoordinateOnItemRequestPacket::setBBoxWanted(bool wanted)
{
   writeBit(FLAGS_POS, BBOX_WANTED_BIT_POS, wanted);
}

inline bool
CoordinateOnItemRequestPacket::getBBoxWanted() const
{
   return readBit(FLAGS_POS, BBOX_WANTED_BIT_POS);
}

inline void
CoordinateOnItemRequestPacket::setAllCoordinatesWanted(bool allWanted)
{
   writeBit(FLAGS_POS, ALL_COORDS_WANTED_BIT_POS, allWanted);
}

inline bool
CoordinateOnItemRequestPacket::getAllCoordinatesWanted() const
{
   return readBit(FLAGS_POS, ALL_COORDS_WANTED_BIT_POS);
}

//---------------
// Reply
//---------------

inline uint32 
CoordinateOnItemReplyPacket::getNbrItems() const {
   return readShort(REPLY_HEADER_SIZE);
}

inline void 
CoordinateOnItemReplyPacket::setNbrItems( uint32 num ) {
   writeShort(REPLY_HEADER_SIZE, num);
}

#endif


