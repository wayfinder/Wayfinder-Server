/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPCACHEELEMENT_H
#define MAPCACHEELEMENT_H

#include "config.h"
#include "CacheElement.h"
#include "NotCopyable.h"

/**
 *    MapCacheElement. Holds a normal graphics map.
 *
 */
class MapCacheElement : public CacheElement, private NotCopyable
{
    public:

      /**
       * Use only for searching!
       *
       * @param mapID the id of the map.
       * @param zoomlevel requested zoomlevel.
       */
      MapCacheElement( uint32 mapID, 
                       CACHE_ELEMENT_TYPE element_type,
                       byte zoomlevel );

      /**
       * Constructs a new MapElement who reads a map from the 
       * mapIP and  mapPort.
       * @param mapID the id of the map.
       * @param element_type is the cache element type.
       * @param hostIP the IP-address to read from.
       * @param hostPort the port of the map.
       * @param zoomlevel requested zoomlevel.
       */
      MapCacheElement( uint32 mapID, 
                       CACHE_ELEMENT_TYPE element_type,
                       uint32 hostIP, 
                       uint16 hostPort,
                       byte zoomlevel );

      MapCacheElement( uint32 mapID, 
                       CACHE_ELEMENT_TYPE element_type,
                       byte zoomlevel,
                       uint32 version,
                       byte* data, 
                       int length );
   
      /**
       * Destructor.
       */
      inline virtual ~MapCacheElement();
   
      /** 
       *    @name Operators
       *    Operators to search the elements in that nice linear way.
       */
      //@{
         /// equal
         bool operator == (const MapCacheElement& el) const;

         /// not equal
         bool operator != (const MapCacheElement& el) const;
      //@}

      /**
       * Get the size of the map data.
       *
       * @return the size of the element.
       */
      inline virtual uint32 getSize() const;

      /**
       * Get the map data.
       *
       * @return a pointer to the map data vector.
       */
      inline byte* getData() const;

      /**
       * @return the zoomlevel of the item.
       */
      inline byte getZoomlevel() const;

      /**
       * @return the map's version. Currently this is UTC time for map creation.
       */
      inline uint32 getVersion() const;
   
   protected:
   
   /**
    * Reads map data from a mapmodule.
    *
    * @param hostIP ip-address of a mapmodule.
    * @param hostPort port of a mapmodule.
    */
   void ReadMap( uint32 hostIP, uint16 hostPort );

   /// The version of the map, currently UTC time of map creation.
   uint32 m_version;

   /// The size of the data buffer.
   uint32 m_size;
    
   /// The data buffer.
   byte* m_data;

   /// The zoomlevel of this elements map.
   byte m_zoomlevel;
};


inline byte MapCacheElement::getZoomlevel() const
{
   return m_zoomlevel;
}


inline uint32 MapCacheElement::getVersion() const
{
   return m_version;
}


inline byte* MapCacheElement::getData() const
{
   return m_data;
}


inline
MapCacheElement::~MapCacheElement() 
{
   delete [] m_data;
}


inline uint32 
MapCacheElement::getSize() const
{
   return m_size + CacheElement::getSize();
}

#endif // MAPCACHEELEMENT_H




