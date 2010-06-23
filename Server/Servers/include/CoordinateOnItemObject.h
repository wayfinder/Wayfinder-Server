/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COORDINATEONITEMOBJECT_H
#define COORDINATEONITEMOBJECT_H

#include "config.h"
#include "CoordinateOnItemPacket.h"
#include "PacketContainerTree.h"
#include "Request.h"
#include <vector>
#include "Vector.h"
#include "MC2BoundingBox.h"

/**
 *    Get the position of a number of items. 
 *
 */
class CoordinateOnItemObject {
   public:
      /**
       * Create new empty object.
       * @param request The Request that uses this object.
       * @param bboxWanted If boundingboxes should be made in the result.
       * @param startSize Initial size of item storage, default 10.
       */
      CoordinateOnItemObject( Request* request, bool bboxWanted = false,
                              uint32 startSize = 10 );


      /**
       * Destructor.
       */
      virtual ~CoordinateOnItemObject();

      
      /**
       *   Add an item to be positioned.
       *   @return Index to use when getting the answer later.
       */
      uint32 add( uint32 mapID, uint32 itemID, uint16 offset );


      /**
       * Get the number of items.
       * @return The number of items.
       */
      inline uint32 getNbrItems() const;


      /**
       * Return a packet container with the next packet that should 
       * be processed by the modules.
       * @return A packet container with the next packet that should
       *         be processed by the modules or NULL if no more packets.
       */
      PacketContainer* getNextPacket();

      
      /**
       * Take care of one packet that is processed by the modules.
       * @param cont A PacketContainer with a packet that have
       *             been processed by the modules.
       */
      void processPacket( PacketContainer* cont );

      
      /**
       * Get the status of the pending request.
       *
       * @return True if all packets are processed, false otherwise.
       */
      inline bool requestDone() const;


      /**
       * Returns if the request is finnished and all went ok.
       *
       * @return True if everything went well, false if not. 
       */
      inline bool finnishedOk() const;


      /**
       * Get the result for item at index index.
       */
      inline void getResult( uint32 index, uint32& mapID, uint32& itemID,
                             uint16& offset, 
                             int32& lat, int32& lon, bool& hasLat ) const;


      /**
       * Get the result for item at index index.
       */
      inline void getResultWithBoundingbox( 
         uint32 index, uint32& mapID, uint32& itemID,
         uint16& offset, int32& lat, int32& lon, bool& hasLat,
         MC2BoundingBox& bbox ) const;


   private:
      /**
       * The possible states of this object.
       */
      enum state_t {
         /**
          * Initial state where items can be added.
          */
         ADDING_ITEMS,

         /**
          * Sending position packets.
          */
         SENDING_COORDINATE_REQUEST,

         /**
          * Awaiting position reply packets.
          */
         AWAITING_COORDINATE_REPLY,

         /**
          * Finnished successfully.
          */
         DONE,

         /**
          * Some error has accured, don't use any result data.
          */
         ERROR } m_state;


      /**
       * The Request that uses this object.
       */
      Request* m_request;


      /**
       * The packets to send.
       */
      PacketContainerTree m_packetsReadyToSend;

      
      /**
       * A input item data and the resulting lat and lon.
       */
      struct inputOutputItem {
         uint32 m_mapID;
         uint32 m_itemID;
         uint16 m_offset;
         uint16 m_index; // Index in the packet with the same mapID
         uint32 m_lat;
         uint32 m_lon;
         bool m_hasLat;
         MC2BoundingBox m_bbox;
      };


      /**
       * A vector of inputOutputItem.
       */
      typedef vector<inputOutputItem> inputOutputItemVector;


      /**
       * The added inputOutputItem.
       */
      inputOutputItemVector m_inputs;

      
      /**
       * Holds the packetIDs for the coordinatepackets.
       */
      Vector m_packetIDs;

      
      /**
       * Holds the mapIDs for the coordinatepackets.
       */
      Vector m_mapIDs;

      
      /**
       * The number of coordinatepackets;
       */
      uint32 m_nbrCoordinatePackets;

      
      /**
       * The number of received coordinatepackets;
       */
      uint32 m_nbrReceivedCoordinatePackets;

      
      /**
       * If boundingboxes are wanted.
       */
      bool m_bboxWanted;
      

      /**
       * Creates Coordinate packets from the m_inputs and adds them to 
       * m_packetsReadyToSend.
       */
      void createCoordinatePackets();


      /**
       * The next packetID for the request.
       */
      inline uint16 getNextPacketID();


      /**
       * The requestID for the Request.
       */
      inline uint16 getRequestID();
};


// ========================================================================
//                                  Implementation of the inlined methods =


inline uint32 
CoordinateOnItemObject::getNbrItems() const {
   return m_inputs.size();
}


inline bool 
CoordinateOnItemObject::requestDone() const {
   return ( m_state == DONE || m_state == ERROR );
}


inline bool
CoordinateOnItemObject::finnishedOk() const {
   return ( m_state == DONE );
}


inline void 
CoordinateOnItemObject::getResult( uint32 index, 
                                   uint32& mapID, 
                                   uint32& itemID,
                                   uint16& offset, 
                                   int32& lat, int32& lon, 
                                   bool& hasLat ) const
{
   mapID = m_inputs[ index ].m_mapID;
   itemID = m_inputs[ index ].m_itemID;
   offset = m_inputs[ index ].m_offset;
   lat = m_inputs[ index ].m_lat;
   lon = m_inputs[ index ].m_lon;
   hasLat = m_inputs[ index ].m_hasLat;
}


inline void 
CoordinateOnItemObject::getResultWithBoundingbox( 
   uint32 index, uint32& mapID, uint32& itemID,
   uint16& offset, int32& lat, int32& lon, bool& hasLat,
   MC2BoundingBox& bbox ) const
{
   getResult( index, mapID, itemID, offset, lat, lon, hasLat );
   if ( m_bboxWanted ) {
      bbox = m_inputs[ index ].m_bbox;
   }
}


inline uint16 
CoordinateOnItemObject::getNextPacketID() {
   return m_request->getNextPacketID();
}


inline uint16 
CoordinateOnItemObject::getRequestID() {
   return m_request->getID();
}


// CoordinateOnItemObject_H
#endif

