/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TOPREGIONPACKET_H
#define TOPREGIONPACKET_H

#include "config.h"
#include "Packet.h"
#include "TopRegionMatch.h"

class ItemIDTree;

#define TOP_REGION_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define TOP_REGION_REPLY_PRIO   DEFAULT_PACKET_PRIO

/**
 *   Packet that is used for asking the MapModule for all top regions.
 *   Cointains nothing but the header.
 *
 */
class TopRegionRequestPacket : public RequestPacket {
   public:
      /**
       *   Create a new TopRegionRequestPacket.
       *   @param reqID unique request ID
       *   @param packetID unique packet ID
       */
      TopRegionRequestPacket( uint16 reqID, uint16 packetID );
};


/**
 *   Packet that is send as reply to the TopRegionRequest-packet.
 *
 *   After the normal header this packet contains
 *   @packetdesc
 *      @row REPLY_HEADER_SIZE @sep 4  bytes @sep Number of top regions 
 *                                           @endrow
 *      @row For each country  @sep 4  bytes @sep Top region type @endrow
 *      @row                   @sep 4  bytes @sep Top region id @endrow
 *      @row                   @sep 16 bytes @sep BoundingBox @endrow
 *      @row                   @sep 4  bytes @sep Number of strings @endrow
 *      @row Nbr of strings *  @sep 4  bytes @sep language_t @endrow
 *      @row Nbr of strings *  @sep 4  bytes @sep name_t @endrow
 *      @row Nbr of strings *  @sep string   @sep String @endrow
 *      @row After the topregs * @sep idtree @sep idtree @endrow
 *   @endpacketdesc
 *
 */
class TopRegionReplyPacket : public ReplyPacket {

public:
      
      /**
       *   Create a TopRegionReplyPacket to the requestpacket given as 
       *   parameter including status.
       *
       *   @param packet is the request packet.
       *   @param status is the status code.
       */
      TopRegionReplyPacket( const TopRegionRequestPacket* packet, 
                            StringTable::stringCode status );
      
      /**
       *   Create a TopRegionReplyPacket to the requestpacket given as 
       *   parameter. Status is set to OK.
       *
       *   @param packet      is the request packet.
       *   @param topRegions  vector containing the top regions.
       */
      TopRegionReplyPacket( const TopRegionRequestPacket* packet,
                            const ItemIDTree& wholeTree,
                            const ConstTopRegionMatchesVector& topRegions );

      /**
       *    Get the top regions.
       *    @param   topRegions  Vector that will be filled with
       *                         the available top regions.
       */
      void getTopRegions( TopRegionMatchesVector& topRegions ) const;

      /**
       *    Get the topregions and the complete idtree.
       *    @param topRegions Vector that will be filled with
       *                      the available top regions.
       *    @param wholeTree  The whole itemIDTree containing all maps
       *                      in index.db.
       */
      void getTopRegionsAndIDTree( TopRegionMatchesVector& topRegions,
                                   ItemIDTree& wholeTree) const;



      /**
       * @return Returns the size this packet needs when written in a 
       *         data buffer. Returns the exact size or larger size.
       */
      uint32 getSizeInDataBuffer( const ConstTopRegionMatchesVector& topRegions,
                                  const ItemIDTree& wholeTree ) const;


         
  private:

      /**
       *   Get the topregions and return the current position in packet.
       */
      int privateGetTopRegions( TopRegionMatchesVector& topRegions ) const;
      
};


#endif

