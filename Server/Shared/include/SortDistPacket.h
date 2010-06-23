/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SORTDISTPACKET_H
#define SORTDISTPACKET_H

#include "Packet.h"
#include "Types.h"
#include "config.h"

#define SORTDIST_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define SORTDIST_REPLY_PRIO   DEFAULT_PACKET_PRIO


/**
 *    Packet that takes one origin (must be the first item)
 *    and sort the destinations (the rest of the items) after the distance to 
 *    the origin.
 */
class SortDistanceRequestPacket : public RequestPacket 
{
	public:
		
      /** 
       * Constructor.
       */
      SortDistanceRequestPacket();

		/**
		 * @return the number of items added to the packet.
		 */
		int getNbrItems();

      /**
       * Adds an item to the packet.
       *
       * @param mapID is the map ID of the item.
       * @param itemID is the itemID of the item.
       * @param offset is the offset of the item.
       */
      void add( uint32 mapID, uint32 itemID, uint16 offset );

      /**
       * Returns all the data for an item.
       *
       * @param index is the index of the item.
       * @param mapID is the map ID of the item.
       * @param itemID is the itemID of the item.
       * @param offset is the offset of the item.
       */
      void getData( int index, uint32& mapID, uint32& itemID, uint16& offset );
};

/**
 */
class SortDistanceReplyPacket : public ReplyPacket 
{
   public:

      /**
       * Constructor.
       *
       */
      SortDistanceReplyPacket();
      
      /** 
       * @param nbr is the total number of items added to the packet.
       */
      void setNbrItems( int nbr );

      /** 
       * @return the total number of items added to the packet.
       */
      int getNbrItems();
      
      /** 
       * Add an item to the packet.
       * 
       * @param index is the index to put the item at in the packet.
       * @param mapID is the mapID of the item.
       * @param itemID is the itemID of the item.
       * @param distance is the distance between the origin and this item.
       * @param sortIndex is the index after sorting.
       */
      void add( int index, uint32 mapID, uint32 itemID, uint32 distance, uint32 sortIndex );

      /** 
       * Get data for an item in the packet.
       * 
       * @param index is the index to put the item at in the packet.
       * @param mapID is the mapID of the item.
       * @param itemID is the itemID of the item.
       * @param distance is the distance between the origin and this item.
       * @param sortIndex is the index after sorting.
       */
      void getData( int index, uint32& mapID, uint32& itemID, uint32& distance, uint32& sortIndex );
};

#endif
