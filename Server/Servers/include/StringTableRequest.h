/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRINGTABLEREQUEST_H
#define STRINGTABLEREQUEST_H 

#include "config.h"
#include "MapPacket.h"
#include "PacketContainer.h"
#include "Request.h"
#include "Cache.h"
#include "StringCacheElement.h"

/**
  *   Request a map from the MapServer.
  *
  */
class StringTableRequest : public Request {
public:
	/**
    * Creates a request who owns and uses a existing MapRequestPacket.
    * @param reqID is a unique requestid.
    * @param p is the MapRequestacket to send.
    * @param cache is the Cache to use, default NULL.
	 */
	StringTableRequest( uint16 reqID, MapRequestPacket* p, Cache* cache);

   /**
    * Creates a request
    * @param reqID is a unique requestid.
    * @param mapType is the type of Map to request.
    * @param mapID is the ID of the Map.
    * @param cache is the Cache to use, default NULL. 
    */
      /*
        MapRequest( uint16 reqID, 
        uint32 mapID, 
        Cache* cache);
      */

   /**
    * Destructs the request
    */
   ~StringTableRequest();


   /**
    * Return the StringTableRequestPacket.
    * @return the StringTableRequestPacket or NULL if allready sent.
    */
	PacketContainer* getNextPacket();


   /**
    * Handle an answer.
    * @param pack is the answer.
    */
	void processPacket(PacketContainer *pack);


   /**
    * Returns NULL allways but must be called before getStringTable.
    * Calls virtual makeStringTableCacheElement to make new StringCacheCacheElements.
    * @return NULL
    */
	PacketContainer* getAnswer();

   
   /**
    * Returns a StringTableCacheElement with the StringTable.
    * NULL if something has gone really wrong.
    */
   StringTableCacheElement* getStringTable();


   /**
    * Makes a StringTableCacheElement.
    * Overload this function to make subclasses of StringTableCacheElements.
    * @param mapID is the ID of the map.
    * @param IP is the IP to get the map from.
    * @param port is the port to connect to.
    * @return A new StringTableCacheElement.
    */
   virtual StringTableCacheElement* makeStringTableCacheElement( uint32 mapID,
                                                                 uint32 IP,
                                                                 uint16 port );

   inline uint32 getMapID();
   
  protected:                                              
   /**
    * @return the cache of this request
    */
   Cache* getCache() { return m_cache; };

  private:
   /// The StringTableRequest to send
	PacketContainer* m_request;
   
   
   /// The answer with IP and Port.
	PacketContainer* m_answer;
   
   /// The StringTable.
   StringTableCacheElement* m_stringTable;

   /// If answer should be fetched.
   bool m_getAnswer;


   /// The ID of the requested map.
   uint32 m_mapID;


   /// The Cache to use
   Cache* m_cache;
};

uint32 StringTableRequest::getMapID()
{
   return m_mapID;
}

#endif // STRINGTABLEREQUEST_H


