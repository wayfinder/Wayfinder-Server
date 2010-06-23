/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COORDINATEONITEMREQUEST_H
#define COORDINATEONITEMREQUEST_H

#include "config.h"
#include "Request.h"
#include "CoordinateOnItemObject.h"


/**
 * Get the position of a number of items. 
 *
 */
class CoordinateOnItemRequest : public Request {
   public:
      /**
       * Create new empty request.
       * @param reqID A unique requestid.
       * @param bboxWanted If boundingboxes should be made in the result.
       */
      CoordinateOnItemRequest( uint16 reqID, bool  bboxWanted = false );


      /**
       * Destructor.
       */
      virtual ~CoordinateOnItemRequest();

      
      /**
       * Add an item to be positioned.
       */
      inline void add( uint32 mapID, uint32 itemID, uint16 offset );


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
      virtual PacketContainer* getNextPacket();

      
      /**
       * Take care of one packet that is processed by the modules.
       * @param cont A PacketContainer with a packet that have
       *             been processed by the modules.
       */
      virtual void processPacket( PacketContainer* cont );


      /**
       * Get the status of the pending request.
       *
       * @return True if all packets are processed, false otherwise.
       */
      bool virtual requestDone();

      
      /**
       * DO NOT USE THIS FUNCTION! It always returns NULL.
       * @return NULL.
       */
      virtual PacketContainer* getAnswer();


      /**
       * Get the result for item at index index.
       */
      inline void getResult( uint32 index, uint32& mapID, uint32& itemID,
                             uint16& offset, 
                             int32& lat, int32& lon, bool& hasLat ) const;


      /**
       * Get the result for item at index index, including the boundingbox.
       */
      inline void getResultWithBoundingbox( 
         uint32 index, uint32& mapID, uint32& itemID,
         uint16& offset, int32& lat, int32& lon, bool& hasLat,
         MC2BoundingBox& bbox ) const;


   private:
      /**
       * The object that handles it all.
       */
      CoordinateOnItemObject m_handler;
};


// ========================================================================
//                                  Implementation of the inlined methods =


inline void 
CoordinateOnItemRequest::add( uint32 mapID, uint32 itemID, 
                              uint16 offset ) 
{
   m_handler.add( mapID, itemID, offset );
}


inline uint32 
CoordinateOnItemRequest::getNbrItems() const {
   return m_handler.getNbrItems();
}


inline void 
CoordinateOnItemRequest::getResult( uint32 index, 
                                    uint32& mapID, 
                                    uint32& itemID,
                                    uint16& offset, 
                                    int32& lat, int32& lon, 
                                    bool& hasLat ) const
{
   m_handler.getResult( index, mapID, itemID, offset, lat, lon, hasLat );
}


inline void 
CoordinateOnItemRequest::getResultWithBoundingbox( 
   uint32 index, uint32& mapID, uint32& itemID,
   uint16& offset, int32& lat, int32& lon, bool& hasLat,
   MC2BoundingBox& bbox ) const
{
   m_handler.getResultWithBoundingbox( 
      index, mapID, itemID, offset, lat, lon, hasLat, bbox );
}


// COORDINATEONITEMREQUEST_H
#endif

