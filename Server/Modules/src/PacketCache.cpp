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

#include "PacketCache.h"
#include "DeleteHelpers.h"

#include "Properties.h"
#include "MC2CRC32.h"
#include "Packet.h"

#include <unistd.h>

PacketCacheKey::PacketCacheKey(uint32 mapID,
                               uint32 requestType,
                               uint32 requestLength,
                               uint32 requestCRC)
      : m_mapID(mapID),
        m_requestType(requestType),
        m_requestLength(requestLength),
        m_requestCRC(requestCRC)
{
}

bool
PacketCacheKey::operator<(const PacketCacheKey& other) const
{
   if ( m_mapID < other.m_mapID ) {
      return true;
   } if ( m_mapID > other.m_mapID ) {
      return false;
   }
     
   if ( m_requestCRC < other.m_requestCRC ) {
      return true;
   } if ( m_requestCRC > other.m_requestCRC ) {
      return false;
   }
   
   // CRC:s are equal   
   if ( m_requestType < other.m_requestType ) {
      return true;
   } else if ( m_requestType > other.m_requestType ) {
      return false;
   }
   
   //  CRCs are equal and so are the request types
   return m_requestLength < other.m_requestLength;
   
}

uint32
PacketCacheKey::getMapID() const
{
   return m_mapID;
}

PacketCacheValue::PacketCacheValue(const Packet* pack,
                                   const char* packetInfo)
{
   const char* extraString = "(cached)";
   m_packet = pack->getClone(false);

   // Copy the info string and add cached.
   m_packetInfo = new char[strlen(packetInfo)+strlen(extraString)+1];
   strcpy(m_packetInfo, packetInfo);
   strcat(m_packetInfo, extraString);

   // Set the times.
   m_lastUsedTime = TimeUtility::getCurrentTime();
   m_createdTime  = m_lastUsedTime;
}

PacketCacheValue::~PacketCacheValue()
{
   delete m_packet;
   delete [] m_packetInfo;
}

Packet*
PacketCacheValue::getPacketClone() const
{
   return m_packet->getClone(false);
}

uint32
PacketCacheValue::getPacketLength() const
{
   return m_packet->getLength();
}

void
PacketCacheValue::updateLastUsedTime()
{
   m_lastUsedTime = TimeUtility::getCurrentTime();
}

void
PacketCacheValue::setLastUsedTime(uint32 time)
{
   m_lastUsedTime = TimeUtility::getCurrentTime() - time;
}

uint32
PacketCacheValue::getCreationTime() const
{
   return m_createdTime;
}

void
PacketCacheValue::setTimedCacheRef( PacketCache::timedCache_t::iterator timeRef )
{
   m_timeRef = timeRef;
}

PacketCache::timedCache_t::iterator 
PacketCacheValue::getTimedCacheRef()
{
   return m_timeRef;
}

void
PacketCacheValue::putPacketInfo(char* packetInfo) const
{
   strcpy(packetInfo, m_packetInfo);
}

PacketCache::PacketCache()
{
   // Max age 5 minutes or the value in props.
   m_maxAge  = 1000 * Properties::getUint32Property("PACKET_CACHE_MAX_AGE_SEC",
                                                    5*60);
   // Max packet size 20 megs or the value in props.
   m_maxSize = Properties::getUint32Property("PACKET_CACHE_MAX_SIZE_BYTES",
                                             20*1024*1024);
   // Now there are 0 bytes of packets in the cache.
   m_totalPacketSize = 0;
}

PacketCache::~PacketCache()
{
   STLUtility::deleteAllSecond( m_packetCache );
}

uint32
PacketCache::calcCRC(const Packet* packet)
{
   if ( packet == NULL ) return MAX_UINT32;
   
   if ( packet->getBuf() == NULL ) {
      mc2log << warn << "[PacketCache]: Packet has buf == NULL - why?" << endl;
   }
   // FIXME: This could be different for different packets.
   int startOffset = HEADER_SIZE;
   int length      = packet->getLength() - startOffset;
   const byte* bufPtr = packet->getBuf() + startOffset;
   if ( length > 0 ) {
      return MC2CRC32::crc32(bufPtr, length);
   } else {
      return MAX_UINT32;
   }
}

void
PacketCache::remove(cacheMap_t::iterator it)
{
   // Downdate the size
   m_totalPacketSize -= it->second->getPacketLength();
   // Delete the value element
   m_timeCache.erase( it->second->getTimedCacheRef() );
   delete it->second;
   m_packetCache.erase( it );
}

void
PacketCache::handleSpecialRequests(const RequestPacket* request)
{
   // Packets that invalidate the cache.
   switch ( request->getSubType() ) {
      case Packet::PACKETTYPE_ADDDISTURBANCEREQUEST:
      case Packet::PACKETTYPE_REMOVEDISTURBANCEREQUEST:
      case Packet::PACKETTYPE_UPDATEDISTURBANCEREQUEST:
      case Packet::PACKETTYPE_DISTURBANCEPUSH: {
         const uint32 mapID = request->getMapID();
         // Remove all replies on the same map
         for( cacheMap_t::iterator it = m_packetCache.begin();
              it != m_packetCache.end();
              /**/ ) {
            if ( it->first.getMapID() == mapID) {
               mc2dbg << "[PacketCache]: Push removes reply map = "
                      << MC2HEX(mapID)
                      << endl;
               remove(it++);
            } else {
               ++it;
            }
         }
      }
      break;
      default:
      break;
   }
}

inline void
PacketCache::handleInsertNewCachePacketTime( const RequestPacket* request,
                                             const Packet* reply,
                                             const char* packetInfo,
                                             const uint32 crc ) {

   // Will work as a soft limit for the cache.
   int nbrOldestRemoved = 0;
   if ( m_totalPacketSize + reply->getLength() > m_maxSize ) {              
      nbrOldestRemoved = removeOld();
   }
   if ( nbrOldestRemoved ) {
      mc2dbg << "[PacketCache]: Removed " << nbrOldestRemoved
         << " oldest packets - size exceeded" << endl;
   }

   // Now try to add it.
   PacketCacheValue* value = new PacketCacheValue(reply, packetInfo);
   pair<cacheMap_t::iterator,bool> add =
      m_packetCache.insert(make_pair(
               PacketCacheKey(request->getMapID(),
                  request->getSubType(),
                  request->getLength(),
                  crc),
               value));

   if ( ! add.second ) {
      // This should not be possible.
      mc2dbg << "[PacketCache]: Already found same crc"
         << endl;               
      // Not added
      delete value;
      remove(add.first);               
   } else {
      // Added
      m_totalPacketSize += reply->getLength();
      // Add to timedCache_t
      value->setTimedCacheRef( m_timeCache.insert( add.first ) );
   }
}

void
PacketCache::putPacket(const RequestPacket* request,
                       const Packet* reply,
                       const char* packetInfo)
{
   // Check for special requests
   handleSpecialRequests(request);
   
   if ( reply == NULL || reply->getBuf() == NULL ) {
      // The processor probably returned null.
      return;
   }

   // Caching is off
   if ( m_maxSize == 0 || m_maxAge == 0 ) {
      return;
   }
   
   // Unallowed replypackets
   switch ( reply->getSubType() ) {
      case Packet::PACKETTYPE_ACKNOWLEDGE:
         return;
      default:
         break;
   }
   
   // Allowed reqpackets - typically packets that do not have a special
   // effect apart from the answer itself.
   switch ( request->getSubType() ) {
      case Packet::PACKETTYPE_EXPANDROUTEREQUEST:
      case Packet::PACKETTYPE_EDGENODESREQUEST:
      case Packet::PACKETTYPE_GFXFEATUREMAP_IMAGE_REQUEST:
      case Packet::PACKETTYPE_GFXFEATUREMAPREQUEST:
      case Packet::PACKETTYPE_IDTRANSLATIONREQUEST:         
      case Packet::PACKETTYPE_MATCHINFOREQUEST:
      case Packet::PACKETTYPE_SEARCHREQUEST:
      case Packet::PACKETTYPE_SEARCHEXPANDITEMREQUEST:
      case Packet::PACKETTYPE_SUBROUTEREQUEST:      
      case Packet::PACKETTYPE_TILEMAP_REQUEST:      
      {
         uint32 crc = calcCRC(request);
         if ( crc != MAX_UINT32 ) {
            handleInsertNewCachePacketTime( request, reply, packetInfo, crc);
         }
      }
      break;
      default:
      return;       
   }
}



uint32
PacketCache::removeOld()
{
   uint32 limit = TimeUtility::getCurrentTime() - m_maxAge;

   // Find the lower bound of the time limit in the sorted set, all packets
   // older then the limit time will be deleted.
   // If there is no old packets we will delete a percentage of the oldest
   // packets.
   // This will apply to the number of packets and not size.
   timedCache_t::iterator endDelete = 
      lower_bound( m_timeCache.begin(), m_timeCache.end(), 
            limit, TimedCacheCmp() );
   uint32 packetsRemoved = 0;
   if ( endDelete != m_timeCache.begin() ) {
      timedCache_t::iterator startDelete = m_timeCache.begin();
      for ( ; startDelete != endDelete; ++startDelete ) {
         deletePacket( startDelete );
         ++packetsRemoved;
      }
      m_timeCache.erase( m_timeCache.begin(), endDelete );
   } else {
      packetsRemoved = 
         getTotalNbrOfPackets() *
         Properties::getUint32Property( "PACKET_CACHE_REMOVE_PERCENTAGE", 20 ) / 100;
      timedCache_t::iterator startDelete = m_timeCache.begin();
      for ( uint32 i = 0; i != packetsRemoved; ++i, ++startDelete ) {
         deletePacket( startDelete );
      }
      m_timeCache.erase( m_timeCache.begin(), startDelete );
   }
   MC2_ASSERT( m_timeCache.size() == m_packetCache.size() );
   return packetsRemoved;
}

Packet*
PacketCache::getCachedReply(const RequestPacket* request,
                            char* packetInfo)
{
   const uint32 crc = calcCRC(request);
   if ( crc == MAX_UINT32 ) {
      mc2dbg << "[PacketCache]: Could not calc checksum" << endl;
      return NULL;
   }
   
   cacheMap_t::iterator it =
      m_packetCache.find(PacketCacheKey(request->getMapID(),
                                        request->getSubType(),
                                        request->getLength(), crc) );
   
   if ( it != m_packetCache.end() ) {
      const uint32 maxTime = it->second->getCreationTime() + m_maxAge;
      if ( TimeUtility::getCurrentTime() < maxTime ) {
         // Update last used-time
         it->second->updateLastUsedTime();
         
         // Set some of the important fields in the header.
         ReplyPacket* replyPacket = static_cast<ReplyPacket*>
            (it->second->getPacketClone());
         it->second->putPacketInfo( packetInfo );

         replyPacket->setRequestTag( request->getRequestTag() );
         replyPacket->setOriginIP( request->getOriginIP() );
         replyPacket->setOriginPort( request->getOriginPort() );
         replyPacket->setPacketID( request->getPacketID() );
         replyPacket->setRequestID( request->getRequestID() );
         replyPacket->setDebInfo( request->getDebInfo() );

         return replyPacket;
      } else {
         mc2dbg << "[PacketCache]: Packet found but too old" << endl;
         remove(it);
         return NULL;
      }
   } else {
      return NULL;
   }
}


bool 
PacketCache::TimedCacheCmp::operator()( const cacheMap_t::iterator& a, 
      const cacheMap_t::iterator& b ) const {
   return (*a).second->getLastUsedTime() < (*b).second->getLastUsedTime();
}

bool 
PacketCache::TimedCacheCmp::operator()( const cacheMap_t::iterator& a, 
      uint32 b ) const {
   return (*a).second->getLastUsedTime() < b;
}
bool 
PacketCache::TimedCacheCmp::operator()( uint32 b, 
      const cacheMap_t::iterator& a ) const {
   return b > (*a).second->getLastUsedTime();
}
