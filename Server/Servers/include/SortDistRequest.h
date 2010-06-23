/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SORTDISTANCEREQUEST_H
#define SORTDISTANCEREQUEST_H

#include "Types.h"
#include "config.h"
#include "SortDistPacket.h"
#include "CoordinateOnItemPacket.h"
#include "Request.h"
#include "VectorElement.h"
#include "Vector.h"

/**
 *    Request that takes an origin and destinations and sort
 *    the destinations in respect of the distance to the origin.
 */
class SortDistRequest : public Request {

	public:
		/**
		 * Constructor.
		 *
		 * @param reqID is the request ID.
		 * @param sortPacket is the packet containing items to sort.
		 */
		SortDistRequest( uint16 reqID, SortDistanceRequestPacket* sortPacket );

		/**
 		 *  Destructs the request
		 */
		virtual ~SortDistRequest();

	   /**
		 * @return the next packet to send or NULL if nothing to send.
		 */
	    PacketContainer* getNextPacket();

	   /**
		 * Handle an answer from the map module.
		 *
		 * @param packetContainer is the answer from the map module.
		 */
	    void processPacket( PacketContainer *packetContainer );   

	   /**
		 * @return a SortDistanceReplyPacket or null if not possible to sort.
		 */
	    PacketContainer* getAnswer();	

   private:

      /** 
       * The number of packets to be sent/received. 
       */
      int m_nbrPackets;

      /** 
       * The number of packets sent.
       */
      int m_sentPackets;

      /**
       * The number of packets currently received.
       */
      int m_receivedPackets;

      /** 
       * A pointer to vectors with packets to be sent
       */
      CoordinateOnItemRequestPacket** m_coordinatesRequestPackets;

      /**
       * A pointer to a vector with received packets.
       */
      CoordinateOnItemReplyPacket** m_coordinatesReplyPackets;

      /** 
       * A vector containing the mapIDs
       */
       Vector m_mapIDVect;
};

/**
 * A class used by the request to sort by the distance.
 */
class DistObj 
{
   public:
      
      /** 
       * Constructor.
       * 
       * @param index is the index before sorting.
       * @param mapID is the mapID 
       * @param itemID is the itemID
       * @param distance is the distance to sort after.
       */
      DistObj( int index, uint32 mapID, uint32 itemID, uint32 distance );

      /**
       * @return the index before sorting.
       */
      inline int getIndex();

      /**
       * @return the mapID.
       */
      inline uint32 getMapID();

      /**
       * @return the itemID.
       */
      inline uint32 getItemID();

      /**
       * @return the distance.
       */
      inline uint32 getDistance();

      /**
       * @param elm the other instace to compare to.
       * @return true if the distances are equal.
       */
      bool operator == (const DistObj& elm) const {
         return ( m_distance == elm.m_distance );
      }

      /**
       * @param elm the other instace to compare to.
       * @return true if the distances aren't equal.
       */
      bool operator != (const DistObj& elm) const {
         return ( m_distance != elm.m_distance);
      }

      /**
       * @param elm the other instace to compare to.
       * @return true if this instance's distances is greater.
       */
      bool operator > (const DistObj& elm) const {
         return ( m_distance > elm.m_distance);
      }

      /**
       * @param elm the other instace to compare to.
       * @return true if this instance's distances is lesser.
       */
      bool operator < (const DistObj& elm) const {
         return ( m_distance < elm.m_distance);
      }

   private:

      /**
       * Is the index before sorting.
       */
      int m_index;

      /**
       * Is the mapID.
       */
      uint32 m_mapID;

      /**
       * Is the itemID.
       */
      uint32 m_itemID;

      /**
       * Is the distance to sort after.
       */
      uint32 m_distance;

};

inline int DistObj::getIndex()
{
   return m_index;
}

inline uint32 DistObj::getMapID()
{
   return m_mapID;
}

inline uint32 DistObj::getItemID()
{
   return m_itemID;
}

inline uint32 DistObj::getDistance()
{
   return m_distance;
}


#endif

