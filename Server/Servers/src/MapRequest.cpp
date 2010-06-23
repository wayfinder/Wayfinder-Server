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

#include "MapRequest.h"

#define MAX_NBR_RETRIES_FOR_MAPNOTFOUND 3
#define MAP_REQUEST_TIMEOUT    (PacketContainer::defaultResendTimeoutTime)
#define MAP_REQUEST_RESEND_NBR 7

MapRequest::MapRequest( uint16 reqID, 
                        MapRequestPacket* p,
                        CacheElement::CACHE_ELEMENT_TYPE element_type,
                        moduletype_t moduletype,
                        Cache* cache ) 
      : Request( reqID )
{
   DEBUG4( cout << "MapRequestType = " << p->getMapType() << endl );

   m_packetCopy = NULL;
   m_cache = cache;
   m_elementType = element_type;
   m_moduleType = moduletype;
   m_nbrRetries = 0;
   
   if( m_cache != NULL ) {
      MapCacheElement*
         keyElm = new MapCacheElement( p->getMapID(),
                                       element_type,
                                       p->getZoomlevel() );

      m_map = (MapCacheElement*)m_cache->find( keyElm );
      delete keyElm;
   } else {
      m_map = NULL;
      DEBUG4(cerr << "Cache == NULL " << endl);
   }
      
   if ( m_map != NULL ) {
      DEBUG4(cerr << "m_map != NULL - in cache ID = "
                  << p->getMapID() << endl);
      m_done = true;
      delete p;
      p = NULL;
      m_getAnswer = false;
   } else {
      DEBUG4( cerr << "m_map == NULL - not in cache ID = " << p->getMapID()
                   << " zoomlevel = " << uint32(p->getZoomlevel()) << endl );
      m_getAnswer = true;
      m_mapID = p->getMapID();
      m_zoomlevel = p->getZoomlevel();
      p->setRequestID(reqID);
      m_packetID = getNextPacketID();
      p->setPacketID( m_packetID  );
      m_request = new PacketContainer( p,
                                       0,
                                       0,
                                       moduletype,
                                       MAP_REQUEST_TIMEOUT,
                                       MAP_REQUEST_RESEND_NBR);
      m_packetCopy = p->getClone();
   }
   m_answer = NULL;
}

MapRequest::~MapRequest() 
{
   delete m_answer;
   delete m_packetCopy;
}

PacketContainer* 
MapRequest::getNextPacket() 
{
   DEBUG8(cerr << "getNextPacket sending " << m_request << endl;);
   PacketContainer* container = m_request;
   m_request = NULL;
   return container;
}

void 
MapRequest::processPacket( PacketContainer* pack ) 
{
   DEBUG8(cerr << "MapRequest::processPacket " << endl;);
	m_answer = pack;
   if ( m_answer != NULL &&
        m_answer->getPacket()->getPacketID() == m_packetID ) {

      ReplyPacket* rp = static_cast<ReplyPacket*>( m_answer->getPacket() );
      if ( rp->getStatus() == StringTable::MAPNOTFOUND) {
         // Maybe the map isn't loaded yet. Or moved to another module.
         // The next request will probably be after the loadmap from the
         // leader and will have a higher chance to succeed.
         if ( m_nbrRetries++ < MAX_NBR_RETRIES_FOR_MAPNOTFOUND ) {
            char debugString[1024];
            sprintf(debugString, " Map (%d) not found - retrying %d of %d",
                    m_mapID, m_nbrRetries, MAX_NBR_RETRIES_FOR_MAPNOTFOUND);
            mc2dbg << here << debugString << endl;
            // Send a new request.
            Packet* p = m_packetCopy->getClone();
            p->setRequestID( getID() );
            m_packetID = getNextPacketID();
            p->setPacketID( m_packetID );
            m_request = new PacketContainer( p,
                                             0,
                                             0,
                                             m_moduleType,
                                             MAP_REQUEST_TIMEOUT,
                                             MAP_REQUEST_RESEND_NBR);      
            delete m_answer;
            m_answer = NULL;
            m_done = false;
         } else {
            mc2dbg << here << " - Too many retries for map " <<
                   m_mapID << endl;
            m_done = true;
            m_getAnswer = false;
            delete m_answer;               
            m_answer = NULL;
         }
      } else {
         if ( m_nbrRetries > 0 )
            mc2dbg << here << " - Got ok packet after retry number "
                   << m_nbrRetries << endl;
         m_done = true;
         m_getAnswer = true;
      }
   } else {
      mc2dbg << here << " - Faulty answer" << endl;
      delete m_answer;
      m_answer = NULL;
      m_getAnswer = false;
      m_done = true;
   }

}

PacketContainer* 
MapRequest::getAnswer() 
{
   // Connect if nessesary   
   mc2dbg4 << "MapRequest::getAnswer(): top" << endl;
   if( m_getAnswer && (m_answer != NULL) ) {
      mc2dbg4 << "MapRequest::getAnswer(): Connect and get" << endl;
      // Connect and get
      MapReplyPacket* reply = (MapReplyPacket*) m_answer->getPacket(); 
      mc2dbg4 << "MapRequest::getAnswer(): ID " << hex << m_mapID << dec 
              << " type " << uint32(m_elementType) << " zoomlevel " 
              << uint32(m_zoomlevel) << endl;

      MapCacheElement* mapEl 
         = new MapCacheElement( m_mapID, 
                                m_elementType,
                                reply->getReplyIP(), 
                                reply->getReplyPort(), 
                                m_zoomlevel );
      if(!mapEl->isValid() ) {
         // Cache element is not valid, will return null
         mc2log << warn << "MapRequest::getAnswer(): MapCacheElement is not "
                   "valid" << endl;
         m_map = NULL;
         delete mapEl;
      } 
      else {
         mc2dbg4 << "MapRequest::getAnswer(): Element is valid" << endl;
         // Add and lock the gfxelement to the cache
         m_map = dynamic_cast<MapCacheElement*>(m_cache->addAndLock( mapEl ));
         m_getAnswer = false;
      }
   }
	return NULL;
}


MapCacheElement* 
MapRequest::getMap() 
{
   DEBUG8(if ( m_getAnswer ) 
          cerr << "Error: Should get map but didn't!!" << endl;);
   return m_map;
}

/*
GfxCacheElement* MapRequest::makeGfxCacheElement( uint32 mapID,
                                                  byte zoomlevel,
                                                  uint32 IP,
                                                  uint16 port )
{
   if (m_readVersion) {
      cout << "m_readVersion = true!!!" << endl;
   } else {
      cout << "m_readVersion = false!!!" << endl;
   }
   return new GfxCacheElement(mapID, IP, port, 
                              m_cache->getType(), 
                              zoomlevel,
                              m_readVersion);
}

  MapRequest::MapRequest( uint16 reqID, 
                        uint32 mapID,
                        byte zoomlevel,
                        Cache* cache) : Request(reqID)
{
   m_mapID = mapID;
   m_cache = cache;
   m_zoomlevel = zoomlevel;
   m_readVersion = false;

   GfxCacheElement* keyElm = new GfxCacheElement( m_mapID,
                                                  m_zoomlevel,
                                                  m_cache->getType() );

   m_map = (GfxCacheElement*)m_cache->find( keyElm );

   delete keyElm;
   
   if ( m_map != NULL ) {
      m_done = true;
      m_getAnswer = false;
   }
   else {
      m_getAnswer = true;
      m_packetID = getNextPacketID();
      m_request = new PacketContainer(
         new MapRequestPacket(getID(),
                              m_packetID,
                              MapRequestPacket::MAPREQUEST_GFX,
                              m_mapID,
                              m_zoomlevel),
         0,
         0,
         MODULE_TYPE_MAP );
   }
   m_answer = NULL;  
}

*/
