/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAPBUFFERHOLDER_H
#define TILEMAPBUFFERHOLDER_H

#include "config.h"

class BitBuffer;
class TileMap;
class ServerTileMapFormatDesc;
class Packet;

/**
 *   A class containing a tilemap buffer 
 *   and some other needful info about the tilemap.
 */
struct TileMapBufferHolder {
public:   
      
   /// The TileProcessor may tamper with me.
   friend class TileProcessor;

   /**
    *    Constructor with some params.
    *    XXX: The supplied buffer will not be deleted by the
    *    destructor. In case it should be deleted, 
    *    deleteBuffer() must be called.
    */
   TileMapBufferHolder( const char* desc,
                        BitBuffer* buf,
                        uint32 crc,
                        bool empty );

   /**
    *    Constructor with only buffer.
    *    XXX: The supplied buffer will not be deleted by the
    *    destructor. In case it should be deleted, 
    *    deleteBuffer() must be called.
    */
   TileMapBufferHolder( const char* desc,
                        BitBuffer* buf,
                        const ServerTileMapFormatDesc* mapDesc );

   /**
    * Constructor from packet.
    * Calls load.
    */
   TileMapBufferHolder( const Packet* p, int& pos );

   TileMapBufferHolder( const TileMapBufferHolder& other );

   ~TileMapBufferHolder();
   
   /// Get if the tilemap is empty.
   bool getEmpty() const;

   /// Get the crc of the tilemap.
   uint32 getCRC() const;

   /// Get the desc of the tilemap.
   inline const char* getDesc() const;

   /// Get the buffer.
   inline BitBuffer* getBuffer() const;

   /// Delete the buffer.
   void deleteBuffer();
   
   inline bool isGood() const;
   inline void setGood( bool newValue );
   
   TileMapBufferHolder& operator = ( const TileMapBufferHolder& other );
   /// copies other
   /// @param other the object to copy
   /// @param skipDesc whether or not the copy should skip copying the description
   void copy( const TileMapBufferHolder& other, bool skipDesc = false );

   const ServerTileMapFormatDesc* getMapDesc() const { return m_mapDesc; }
   void setMapDesc( const ServerTileMapFormatDesc* m ) { m_mapDesc = m; }
   /// Force a new desc
   void setDesc( const char* desc ) {
      m_desc = desc;
   }

   /**
    * Load from packet.
    */
   void load( const Packet* p, int& pos );

   /**
    * Save to packet.
    */
   void save( Packet* p, int& pos ) const;

   /**
    * @return Size in packet. 
    */
   uint32 getSizeInPacket() const;

private:     

   TileMapBufferHolder(); 

   /// Update the attributes if needed.
   void updateAttributesIfNeeded();
    
   /// Pointer to the tilemap param.
   const char* m_desc;

   /// The TileMap buffer.
   BitBuffer* m_buf;

   /// The crc of the TileMap.
   uint32 m_crc;

   /// If the TileMap is empty or not.
   bool m_empty;
      
   /// True if the map should be sent to the client.
   bool m_good;

   /// True if m_crc and m_empty is to be trusted.
   bool m_trustAttributes;

   /**
    *    Pointer to a mapformatdesc.
    *    Only needed when a TileMapBufferHolder is created with a 
    *    buffer alone, and the m_crc and m_empty are not set.
    *    Can otherwise be NULL.
    */
   const ServerTileMapFormatDesc* m_mapDesc;
};


inline const char* 
TileMapBufferHolder::getDesc() const
{
   return m_desc;
}


inline BitBuffer* 
TileMapBufferHolder::getBuffer() const
{
   return m_buf;
}
      
inline bool
TileMapBufferHolder::isGood() const 
{
   return m_good;
}

inline void
TileMapBufferHolder::setGood(bool newValue) 
{
   m_good = newValue;
}

#endif

