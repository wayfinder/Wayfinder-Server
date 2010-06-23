/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPREQUEST_H
#define MAPREQUEST_H

#include "config.h"
#include "MapPacket.h"
#include "PacketContainer.h"
#include "Request.h"
#include "Cache.h"
#include "MapCacheElement.h"

/**
  *   Request a map from the MapServer
  */
class MapRequest : public Request 
{
   public:
      /**
       * Creates a request who owns and uses a existing MapRequestPacket.
       * @param reqID is a unique requestid.
       * @param p is the MapRequestacket to send.
       * @param cache is the Cache to use, default NULL.
       * @param moduletype optional parameter to specify the module to send to.
       */
      MapRequest( uint16 reqID, 
                  MapRequestPacket* p, 
                  CacheElement::CACHE_ELEMENT_TYPE element_type,
                  moduletype_t moduletype,
                  Cache* cache );

      /**
       * Destructs the request
       */
      virtual ~MapRequest();

      /**
       * Return the MapRequestPacket.
       * @return the MapRequestPacket or NULL if allready sent.
       */
      PacketContainer* getNextPacket();


      /**
       * Handle an answer.
       * @param pack is the answer.
       */
      void processPacket(PacketContainer *pack);


      /**
       * Returns NULL allways but must be called before getMap.
       * Calls virtual makeGfxCacheElement to make new GfxCacheElements.
       * @return NULL
       */
      PacketContainer* getAnswer();

      
      /**
       * Returns a GfxCacheElement with the Map.
       * NULL if something has gone really wrong.
       */
      MapCacheElement* getMap();

      inline int getMapID();

      inline byte getZoomLevel();
      
   protected:                                              
      /**
       * @return the cache of this request
       */
      Cache* getCache() { return m_cache; };

   private:
      /// The MapRequest to send
      PacketContainer* m_request;
      
      
      /// The answer with IP and Port.
      PacketContainer* m_answer;
      
      /// The Map.
      MapCacheElement* m_map;

      /// If answer should be fetched.
      bool m_getAnswer;


      /// The ID of the requested map.
      uint32 m_mapID;

      /// The zoomlevel of the requested map.
      byte m_zoomlevel;

      /// The Cache to use
      Cache* m_cache;

      /// The cache type;
      CacheElement::CACHE_ELEMENT_TYPE m_elementType;

      /**
       *    True if to read the version of the maps, false otherwise.
       */
      bool m_readVersion;

      /**
       *    Copy of the requestpacket.
       */
      Packet* m_packetCopy;

      /**
       *    The module to send the requests to.
       */
      moduletype_t m_moduleType;

      /**
       *    The number of times we have got MAPNOTFOUND for answer.
       */
      int m_nbrRetries;
};

int MapRequest::getMapID(){
   return m_mapID;
}

byte MapRequest::getZoomLevel(){
   return m_zoomlevel;
}
#endif







