/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COORDINATEOBJECT_H
#define COORDINATEOBJECT_H

#include "config.h"
#include "Request.h"
#include "PacketContainerTree.h"
#include "CoordinatePacket.h"
#include "VectorIncl.h"

#define MAX_NBR_COORDINATE_PACKETS 64

class OrigDestInfo;
class TopRegionRequest;


/**
 *    Objects of this class is used to create and handle coordinate 
 *    packets. This class became necessary when the maps started to 
 *    overlap. It means that a given coordinate not belongs to just one
 *    map, but it might be necessary to ask several maps for the closest
 *    item.
 *
 */
class CoordinateObject : public RequestWithStatus {
   public:
      /**
       *    Create a new coordinate object.
       *    @param parentOrID  The request id or the request to get id:s from.
       *    @param   topReq       Pointer to valid TopRegionRequest with data
       *    @param language The prefered language of names.
       */
      CoordinateObject(const RequestData& parentOrID,
                       const TopRegionRequest* topReq,
                       StringTable::languageCode language = 
                       StringTable::ENGLISH);
      
      /**
       *    Destructor that will delete all packets that are allocated
       *    here. This means that even packets that are returned via
       *    the getCoordinateReply(int) will be deleted. These packet
       *    must therefore not be used after this destructor is called!
       */
      ~CoordinateObject();

      /**
       *    Get the next packet to send to the modules.
       *    @return  The next packet to send to the modules. NULL will
       *             be returned if no packet available.
       */
      PacketContainer* getNextPacket();
      
      /**
       *    Handle one packet that has been processed by the modules.
       *    @param pack The packet that has been processed by the map
       *                module.
       */
      void processPacket(PacketContainer *pack);

      /**
       *    Find out if this object is done or not. It is done when
       *    all reply packets have been inserted with the processPacket-
       *    method. The getCoordinateReply(int) could be used to get 
       *    the answers when the object is done.
       *    @return  True if this object is done, false otherwise.
       */
      bool getDone() const;
      
      /**
       *    Add one coordinate to get the closest item for.
       *    @param   lat   The latitude for the coordinate.
       *    @param   lon   The longitude for the coordinate.
       *    @param   outDatatypes Bitfiled with the types of data in the
       *                   reply packet. See flaggs in 
       *                   CoordinateRequestPacket.
       *    @param   nbrAllowedItemTypes The number of elements in the
       *                   itemTypes-array.
       *    @param   itemTypes An array with the item types that are
       *                   allowed in the reply. Contains 
       *                   nbrAllowedItemTypes elements.
       *    @param   angle The heading of the vehicle, in degrees from the
       *                   north direction. This will obly be used when 
       *                   determine the closest segment if the value is
       *                   <= 360 and the itemTypes-array contains one
       *                   element and that is StreetSegmentItem. 
       *    @return  The index of the coordinate, could be used when
       *             getting the results.
       */
      int addCoordinate(int32 lat, int32 lon, 
                        uint32 outDataTypes,
                        byte nbrAllowedItemTypes,
                        const byte* itemTypes,
                        uint16 angle = MAX_UINT16);

      /**
       *    Add an OrigDestInfo to get the closest item for.
       *    The itemID and nodeID of the OrigDestInfo will
       *    not be used and only StreetSegmentItems will be allowed.
       *    @param info The OrigDestInfo to add.
       *    @return The index of the coordinate. Should be used when
       *            getting the result.
       */
      int addOrigDestInfo(OrigDestInfo& info);

      /**
       *    Fill in the coordinates for the origdestinfo at position
       *    <code>index</code>.
       *    @param index The index of the coordinate, the one you got
       *                 when adding.
       *    @param info  The info to fill in. Only the mapid and itemid
       *                 should be changed.
       *    @return <code>index</code> on success. -1 on failure.
       */
      int fillOrigDestInfo(uint32 index,
                           OrigDestInfo& info);
      
      /**
       *    Get a reply. Please notice that the returned packet {\bf must
       *    not} be used after that this object is deleted.
       *    @param   i  The index of the reply to get. If no parameter is
       *                given, the first one will be returned.
       *    @return  A pointer to the packet that contains information 
       *             about the item that is closest to coordinate number 
       *             i. NULL will be returned upon failure.
       */
      CoordinateReplyPacket* getCoordinateReply(uint32 i = 0);

      /**
       *    Returns the current status of the request.
       */
      StringTable::stringCode getStatus() const;

      /**
       *    Returns true when the request is done.
       */
      bool requestDone();

   private:
      /**
       *    Containers with the packets that are ready to be processed
       *    by the modules.
       */
      PacketContainerTree m_packetsReadyToSend;

      /**
       *    Current status of the object.
       */
      StringTable::stringCode m_status;

      class CoordNotice : public VectorElement {
         public:
         CoordNotice(int32 lat, int32 lon, 
                     uint32 outDataTypes,
                     byte nbrItemTypes,
                     const byte* itemTypes,
                     uint16 angle = MAX_UINT16) : m_packetIDs(4) {
            m_lat = lat;
            m_lon = lon;
            m_angle = angle;
            m_minDist = MAX_UINT32;
            m_outDataTypes = outDataTypes;
            m_nbrItemTypes = nbrItemTypes;
            m_itemTypes = new byte[nbrItemTypes];
            memcpy(m_itemTypes, itemTypes, nbrItemTypes);
            m_replyPacket = NULL;
         }
         virtual ~CoordNotice() {
            delete [] m_itemTypes;
            delete m_replyPacket;
         }
         Vector16 m_packetIDs;
         uint32 m_minDist;
         CoordinateReplyPacket* m_replyPacket;
         int32 m_lat, m_lon;
         uint32 m_outDataTypes;
         uint16 m_angle;
         byte m_nbrItemTypes;
         byte* m_itemTypes;
      };
      
      std::vector< CoordNotice* >  m_coordNotices;

      CoordNotice* getNoticeWithPacketID(uint16 pID);
      
      /**
       * The prefered language.
       */
      StringTable::languageCode m_language;

      /// pointer to valid topregionrequest *with* data
      const TopRegionRequest* m_topReq;
};


#endif

