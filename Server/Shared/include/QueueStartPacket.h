/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef QUEUESTARTPACKET_H
#define QUEUESTARTPACKET_H

#include "config.h"
#include "Packet.h"

#define QUEUE_START_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define QUEUE_START_REPLY_PRIO   DEFAULT_PACKET_PRIO


#define QUEUE_START_REQUEST_NODE_ID  0
#define QUEUE_START_REQUEST_DISTANCE 4


/**
  *   Request packet for searching backwards along a road for the
  *   starting point of a traffic queue.
  */
class QueueStartRequestPacket : public RequestPacket {
  public:
      /** 
       *   Creates a new packet and sets the values to default values.
       */
      QueueStartRequestPacket();

      /** 
       *   Creates a new packet and sets the values.
       *   @param firstNodeID The start node (on this map).
       *   @param distance The length of the queue.
       */
      QueueStartRequestPacket(uint32 firstNodeID,
                              uint32 distance);
      
      /**
       *   Return the nodeID.
       *   @return The ID of the start node on this map.
       */
      uint32 getFirstNodeID() const;
         
      /**
       *   Returns the (max)distance searched.
       *   @return The distance of the queue.           
       */
      uint32 getDistance() const;

      /**
       *   Set the ID of the node to start the search on.
       *   @param nodeID  Start node ID.
       */
      void setFirstNodeID(uint32 nodeID);

      /**
       *   Set the distance to be searched.
       *   @param Search distance.
       */
      void setDistance(uint32 distance);
      

      
   private:
};

#define QUEUE_START_REPLY_NODE_ID      0
#define QUEUE_START_REPLY_DISTANCE     4
#define QUEUE_START_REPLY_NEXT_MAP_ID  8

/**
 *  Reply packet with the start node of a traffic queue.
 *
 *  If the nextMapID is not MAX_UINT32 the nodeID is the ID of the
 *  node to continue the search on on the next map.
 */
class QueueStartReplyPacket : public ReplyPacket {
   public:
      /**
        *   Creates an empty QueueStartReplyPacketReplyPacket.
        */
      QueueStartReplyPacket(int size = MAX_PACKET_SIZE);

      /**
        *   Creates a QueueStartReplyPacket as a reply to a given
        *   QueueStartRequestPacket.
        *   @param   p  The corresponding request packet.
        */
      QueueStartReplyPacket(const QueueStartRequestPacket* p);

         
      /**
       *  Set the node id of the reply. This is the final node if
       *  the search is done (nextMapID == MAX_UINT32) or the 
       */
      void setNodeID(uint32 lastNodeID);
         
         
      /**
       *  Set the distance that have been covered on this map.
       *  This might be less than the requested distance if the queue
       *  leavs the map or if we have to give up.
       *  @param passedDistance Distance searched.
       */
      void setDistance(uint32 passedDistance);
         
      /**
       *  The ID of the next map to continue on.
       *  Set to MAX_UINT32 if we are done.
       *  @param nextMapID Id of the next map to search.
       */
      void setNextMapID(uint32 nextMapID);

      /**
       *  True if we are done (nextMapID == MAX_UINT32).
       *  @return True if we are done.
       */
      bool isDone() const;
      
      /**
       *  Return the nodeID of the reply. The final node or a node
       *  on the next map to resume the search on.
       *  @return The nodeID of the reply.
       */
      uint32 getNodeID() const;

      /**
       *  Return the distance covered on this map.
       *  @return The distance covered.
       */
      uint32 getDistance() const;

      /**
       *   Return the ID of the next map to search on.
       *   @return The ID of the next map.
       */
      uint32 getNextMapID() const;
      
   private:
};

#endif
