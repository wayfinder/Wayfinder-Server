/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include <map>
#include <set>

class RequestPacket;
class Packet;
class PacketCacheValue;

class PacketCacheKey {
public:
   /**
    *   Creates a new PacketCacheKey.
    *   @param mapID          Map ID of the packet.
    *   @param requestType    Type of packet as in getSubType.
    *   @param requesttLength The length of the packet.
    *   @param requestCRC     CRC for the packet.
    */
   PacketCacheKey(uint32 mapID,
                  uint32 requestType,
                  uint32 requestLength,
                  uint32 requestCRC);

   /**
    *   To be able to put the key into the map.
    */
   bool operator<(const PacketCacheKey& other) const;

   /**
    *   The mapID of the request.
    */
   uint32 getMapID() const;
   
private:
   /// The map id of the packet.
   uint32 m_mapID;
   /// Type of packet
   uint32 m_requestType;
   /// Length of packet
   uint32 m_requestLength;
   /// CRC for packet
   uint32 m_requestCRC;   
};

/**
 *   
 */
class PacketCache {
public:

   /**
    *   Creates a new PacketCache.
    */
   PacketCache();

   /**
    *   Destructor.
    */
   ~PacketCache();

   /**
    *   Looks at the request and the reply and tries to
    *   save them in the cache.
    */
   void putPacket(const RequestPacket* request,
                  const Packet* reply,
                  const char* packetInfo);

   /**
    *   Returns a copy of a cached packet. The packet
    *   should be deleted by the caller (or something
    *   called by the caller.).
    *   @param request Request to check for in the cache.
    *   @return A reply to the request.
    */
   Packet* getCachedReply(const RequestPacket* request,
                          char* packetInfo);
   
   /**
    *   Returns the current size of the buffer.
    */
   inline uint32 getCurrBufSize() const;

   /**
    *   Returns the number of packets in the buffer.
    */
   inline uint32 getTotalNbrOfPackets() const;

   /// Type of cache-map
   typedef map<PacketCacheKey, PacketCacheValue*> cacheMap_t;

   /// A functor to compare the time to be able to sort the packets after time
   class TimedCacheCmp {
      public:

         bool operator () ( const cacheMap_t::iterator& a, 
               const cacheMap_t::iterator& b ) const;

         bool operator () ( const cacheMap_t::iterator& a, uint32 b ) const;
         bool operator () ( uint32 b, const cacheMap_t::iterator& a ) const;
   };

   /// Container that sorts by last time used
   typedef multiset< cacheMap_t::iterator, TimedCacheCmp > timedCache_t;

private:


   /** 
   * Handles the deleting by sorting a multiset by lastTimeUsed 
   */
   void handleInsertNewCachePacketTime( const RequestPacket* request,
                                        const Packet* reply,
                                        const char* packetInfo,
                                        const uint32 crc );

   /**
    *    Removes the old packets, if there are packet old enough we remove those
    *    else we will empty the cache by a certain percantage value.
    *    The percentage will apply to the number of packets in the cache at the
    *    current time and not based on the size.
    */
   uint32 removeOld();

   /**
   *    Deletes a packet from the cache
   */
   void deletePacket( timedCache_t::iterator start );

   /**
    *   Does special things for some requests, e.g.
    *   DisturbancePushPackets.
    */
   void handleSpecialRequests(const RequestPacket* request);
   
   /**
    *   Calculates the CRC for the supplied packet.
    *   @param packet Packet to calculate checksum for.
    *   @return The checksum. MAX_UINT32 if too short buffer or so.
    *           This means that we cannot cache packet with crc MAX_UINT32.
    */
   static uint32 calcCRC(const Packet* packet);

   /**
    *    Removes the entry from the m_packetCache and updates
    *    other values.
    */
   void remove(cacheMap_t::iterator it);

   /// Cache of old packets
   cacheMap_t m_packetCache;

   /// Cache stored by time
   timedCache_t m_timeCache;

   /// Total size of the packets in the cache.
   uint32 m_totalPacketSize;

   /// Maximum age in milliseconds
   uint32 m_maxAge;

   /// Maximum size of packets in bytes
   uint32 m_maxSize;
   
};

class PacketCacheValue {
public:
   /**
    *   Creates a new PacketCacheValue element.
    *   Copies the packet and the packetInfo. (adds (cached) to the packinf)
    */
   PacketCacheValue(const Packet* packet,
                    const char* packetInfo);

   /**
    *   Deletes the things newed in constructor.
    */
   ~PacketCacheValue();

   /**
    *   Returns a new clone of the packet inside.
    */
   Packet* getPacketClone() const;

   /**
    *   Returns the result of m_packet->getLength().
    */
   uint32 getPacketLength() const;

   /**
    *   Copies the packetInfo-string into the
    *   other string.
    */
   void putPacketInfo(char* packetInfo) const;

   /**
    *   Updates the last used-time.
    */
   void updateLastUsedTime();

   /**
   * Set the last used-time
   */
   void setLastUsedTime(uint32 time);

   /**
    *   Returns last time the value was used.
    */
   inline uint32 getLastUsedTime() const;

   /**
    *   Returns the creation time.
    */
   uint32 getCreationTime() const;

   /**
    *   Sets reference to the timedCache_t iterator that points this
    */
   void setTimedCacheRef( PacketCache::timedCache_t::iterator timeRef );

   /**
    *   Gets reference to the timedCache_t iterator that points this
    */
   PacketCache::timedCache_t::iterator getTimedCacheRef();

private:
   /**
    *   The packet
    */
   Packet* m_packet;
   
   /**      
    *   The packetinfo.
    */
   char* m_packetInfo;

   /**
    *   Time when the element was last used.
    */
   uint32 m_lastUsedTime;

   /**
    *   Time when the element was created.
    */
   uint32 m_createdTime;

   /**
    *  The iterator that points to this in the multiset timedCache_t 
    */
   PacketCache::timedCache_t::iterator m_timeRef;

};

inline void
PacketCache::deletePacket( timedCache_t::iterator start )
{
      m_totalPacketSize -= (*start)->second->getPacketLength();
      delete (*start)->second;
      m_packetCache.erase( *start);
}

inline uint32
PacketCacheValue::getLastUsedTime() const
{
   return m_lastUsedTime;
}

inline uint32
PacketCache::getCurrBufSize() const
{
	return m_totalPacketSize;
}

inline uint32
PacketCache::getTotalNbrOfPackets() const
{
	return m_packetCache.size();
}
