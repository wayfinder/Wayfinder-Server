/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COVEREDIDSREQUEST_H
#define COVEREDIDSREQUEST_H

#include "config.h"
#include "Request.h"
#include "ItemTypes.h"
#include "PacketContainer.h"
#include "MC2Coordinate.h"
#include "IDPairVector.h"

class CoveredIDsRequestPacket;
class CoveredIDsReplyPacket;
class TopRegionRequest;

/**
 *    Request for getting all items (of a certain type) inside
 *    a circlesector. Probably used internally in another request.
 *    Can handle coordinates as origin now, but itemid can easily
 *    be added in createCoveredIDsRequestPacket.
 */
class CoveredIDsRequest : public RequestWithStatus {
public:
   /// Type of storage for returned ids.
   typedef multimap<ItemTypes::itemType, IDPair_t> result_t;
   
   /**
    *   Creates a new CoveredIDsRequest.
    *   @param parentOrID  The request id or the request to get id:s from.
    *   @param center       The center of the circle.
    *   @param radiusMeters Radius of the circle.
    *   @param itemTypes    The itemtypes to look for.
    *   @param topReq       Pointer to valid TopRegionRequest with data
    */
   CoveredIDsRequest(const RequestData& parentOrID,
                     const MC2Coordinate& center,
                     uint32 radiusMeters,
                     const set<ItemTypes::itemType>& itemTypes,
                     const TopRegionRequest* topReq);

   /**
    *   Creates a new CoveredIDsRequest.
    *   @param parentOrID  The request id or the request to get id:s from.
    *   @param centerID     The center item.
    *   @param radiusMeters Radius of the circle.
    *   @param itemTypes    The itemtypes to look for.
    *   @param topReq       Pointer to valid TopRegionRequest with data
    */
   CoveredIDsRequest(const RequestData& parentOrID,
                     const IDPair_t& centerID,
                     uint32 radiusMeters,
                     const set<ItemTypes::itemType>& itemTypes,
                     const TopRegionRequest* topReq);

   /**
    *   Creates a new CoveredIDsRequest.
    *
    *   @param parentOrID  The request id or the request to get id:s from.
    *   @param bbox         The boundingbox to get items in.
    *   @param itemTypes    The itemtypes to look for.
    *   @param topReq       Pointer to valid TopRegionRequest with data
    */
   CoveredIDsRequest(const RequestData& parentOrID,
                     const MC2BoundingBox& bbox,
                     const set<ItemTypes::itemType>& itemTypes,
                     const TopRegionRequest* topReq);


   /**
    *   Returns StringTable::OK if ok.    
    */
   StringTable::stringCode getStatus() const;

   /**
    *    Returns true if the request is done.
    */
   inline bool requestDone();

   /**
    *    Return a packet container with the next packet that
    *    should be processed by the modules. The packet will be
    *    deleted in this function.
    *    @return  A packet container with the next packet that should
    *             be processed by the modules.
    */
   void processPacket(PacketContainer* pack);

   /**
    *    Returns a <b>reference</b> to the internal structure
    *    containing the covered ids returned from the modules.
    */
   inline const result_t& getCoveredIDs() const;
   
private:

   /**
    *    Initializes the Request.
    *    @param center    The center coordinate or invalid if
    *                     centerID should be used.
    *    @param centerID  The id of the center item. If coordinate
    *                     is valid, it will be used instead.
    *    @param radius    The radius to look inside.
    *    @param itemTypes Allowed itemTypes.
    *    @param topReq       Pointer to valid TopRegionRequest with data
    *    @see CoveredIDsRequest::CoveredIDsRequest.
    */
   void init(const MC2Coordinate& center,
             const IDPair_t& centerID,
             uint32 radius,
             const set<ItemTypes::itemType>& itemTypes,
             const TopRegionRequest* topReq);
   
   /**
    *   Handles a CoveredIDsReplyPacket. May enqueue more packets
    *   in the outgoing queue and may set state to new one.
    *   @param pack Packet to handle.
    */
   inline void handleCoveredIDsPacket(const CoveredIDsReplyPacket* packet);

   /**
    *   Creates a CoveredIDsRequestPacket from the member variables
    *   and the supplied mapid.
    */
   CoveredIDsRequestPacket* createCoveredIDsRequestPacket(uint32 mapID) const;

   /**
    *   Puts the packet into the queue. Sets requestid and packetid.
    *   Also increases the m_outstandingPackets variable.
    *   @return The packetID.
    */
   int addPacketForSending(Packet* packet, moduletype_t moduleType);
   
   /**
    *   The internal states
    */
   enum state_t {
      /** Means that the request is sending and receiving packets */
      SENDING_PACKETS = 10,
      /** The request is done and ok */
      DONE            = 20,
      /** The request is done but an error has occured */
      ERROR           = 30
   };

   /// Variable to keep track of the states.
   state_t m_state;

   /// Set of itemtypes to request.
   set<ItemTypes::itemType> m_itemTypes;

   /// Center coordinate
   MC2Coordinate m_center;

   /// Center ID
   IDPair_t m_centerID;

   /// Outer radius in meters
   uint32 m_outerRadiusMeters;

   /// Inner radius in meters
   uint32 m_innerRadiusMeters;

   /// Start angle in degrees
   uint32 m_startAngle;

   /// Stop angle in degrees
   uint32 m_stopAngle;
   
   /// Number of packets to wait for.
   int m_nbrOutstanding;

   /// The result
   result_t m_resultIDs;
   
   /// pointer to valid topregionrequest *with* data
   const TopRegionRequest* m_topReq;
};

// - Inlined functions

inline bool
CoveredIDsRequest::requestDone()
{
   return (m_state == DONE) || (m_state == ERROR);
}

inline StringTable::stringCode
CoveredIDsRequest::getStatus() const
{
   switch ( m_state ) {
      case DONE:
         return StringTable::OK;
      case ERROR:
         return StringTable::NOTFOUND;
      default:
         return StringTable::TIMEOUT_ERROR;
   };
}

inline const CoveredIDsRequest::result_t&
CoveredIDsRequest::getCoveredIDs() const
{
   return m_resultIDs;
}

#endif

